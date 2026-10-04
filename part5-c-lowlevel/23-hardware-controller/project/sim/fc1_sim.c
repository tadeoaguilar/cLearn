// fc1_sim.c: a software model of the FC-1 chip, so the firmware runs on a PC.
//
// The "silicon" is a background thread that watches the register block
// (fc1_sim_memory) the way real hardware watches its bus: it sees commands
// written by the firmware, updates status registers, and runs "interrupt
// handlers" by calling TIMER_IRQHandler / UART_IRQHandler. While the firmware
// holds irq_disable(), interrupts wait, just like on a CPU with interrupts masked.
//
// It also simulates the physical world: a heat source, a temperature sensor,
// a fan with inertia that can stall, a push button and a serial terminal.
//
// Simplifications (fine for learning; a real emulator such as QEMU does better):
//  - Register reads have no side effects, which is why the FC-1 uses explicit
//    W1C/command registers instead of "reading DR clears the flag".
//  - The two threads share registers through volatile 32-bit accesses. Aligned
//    32-bit loads and stores are atomic on x86-64 and AArch64, which is what we rely on.
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fc1.h"
#include "hal.h"

alignas(4096) uint32_t fc1_sim_memory[FC1_PERIPH_SPAN / sizeof(uint32_t)];

// The hardware side may write registers that are read-only for software.
#define HW(reg) (*(volatile uint32_t*)&(reg))

/* --------------------------- interrupt emulation --------------------------- */
static pthread_mutex_t irq_lock = PTHREAD_MUTEX_INITIALIZER;
void irq_disable(void) { pthread_mutex_lock(&irq_lock); }
void irq_enable(void) { pthread_mutex_unlock(&irq_lock); }

static void raise_irq(void (*handler)(void)) {
    pthread_mutex_lock(&irq_lock); // waits while the firmware has interrupts disabled
    handler();
    pthread_mutex_unlock(&irq_lock);
}

static void sleep_us(long us) {
    struct timespec ts = {us / 1000000, (us % 1000000) * 1000};
    nanosleep(&ts, NULL);
}

static atomic_bool firmware_running = true;
bool hal_running(void) { return atomic_load(&firmware_running); }
void wait_for_interrupt(void) { sleep_us(20); }

/* ------------------------------ the world model ---------------------------- */
typedef struct {
    double temp_c, ambient_c, heat_w, rpm;
    bool stalled, button_down;
    uint32_t ms;            // simulated time
    uint32_t timer_us_acc;  // TIMER prescaler
    uint32_t rng;           // deterministic noise
} World;
static World w = {.temp_c = 25.0, .ambient_c = 25.0, .heat_w = 15.0, .rng = 12345};

static char rx_pending[512]; // bytes "typed" into the terminal, waiting to enter the UART
static size_t rx_head, rx_tail;
static pthread_mutex_t rx_lock = PTHREAD_MUTEX_INITIALIZER;

static void type_text(const char* s) {
    pthread_mutex_lock(&rx_lock);
    for (; *s; s++) {
        if ((rx_head + 1) % sizeof rx_pending == rx_tail) break;
        rx_pending[rx_head] = *s;
        rx_head = (rx_head + 1) % sizeof rx_pending;
    }
    pthread_mutex_unlock(&rx_lock);
}

/* ------------------------------- peripherals ------------------------------- */
static void apply_w1c(volatile uint32_t* sr, volatile uint32_t* icr) {
    uint32_t clear = *icr;
    if (clear) {
        *sr &= ~clear;
        *icr = 0;
    }
}

static char tx_line[256];
static size_t tx_len;

// Called very often (every few µs of real time), so the firmware's register
// handshakes complete quickly.
static void service_fast(void) {
    // GPIO: apply BSRR writes to ODR; the LED pin is an output, the button pin an input.
    uint32_t bsrr = HW(GPIO->BSRR);
    if (bsrr) {
        HW(GPIO->ODR) = (GPIO->ODR | (bsrr & 0xFFFFu)) & ~(bsrr >> 16);
        HW(GPIO->BSRR) = 0;
    }
    HW(GPIO->IDR) = (GPIO->ODR & GPIO->DIR) | (w.button_down ? 0u : 1u << GPIO_PIN_BUTTON);

    // ADC: a START command produces a conversion immediately.
    apply_w1c(&HW(ADC->SR), &HW(ADC->ICR));
    if ((ADC->CR & ADC_CR_EN) && (ADC->CMD & ADC_CMD_START)) {
        w.rng = w.rng * 1103515245u + 12345u;
        int noise = (int)((w.rng >> 16) % 7) - 3; // ±3 LSB, like a real sensor
        int raw = (int)(w.temp_c / ADC_MAX_TEMP_C * ADC_FULL_SCALE + 0.5) + noise;
        HW(ADC->DR) = (uint32_t)(raw < 0 ? 0 : raw > (int)ADC_FULL_SCALE ? (int)ADC_FULL_SCALE : raw);
        HW(ADC->SR) |= ADC_SR_EOC;
        HW(ADC->CMD) = 0; // self-clearing
    }

    // UART TX: take the byte, print complete lines.
    apply_w1c(&HW(UART->SR), &HW(UART->ICR));
    apply_w1c(&HW(TIMER->SR), &HW(TIMER->ICR));
    if ((UART->CR & UART_CR_EN) && (UART->CMD & UART_CMD_SEND)) {
        char c = (char)(UART->TXDR & 0xFF);
        HW(UART->CMD) = 0;
        if (c == '\n') {
            printf("uart │ %.*s\n", (int)tx_len, tx_line);
            fflush(stdout);
            tx_len = 0;
        } else if (c != '\r' && tx_len < sizeof tx_line) {
            tx_line[tx_len++] = c;
        }
    }
}

// Called once per simulated millisecond.
static void service_tick(void) {
    const double dt = 0.001;
    w.ms++;

    // TIMER: LOAD is the period in µs; one simulated ms = 1000 µs.
    if (TIMER->CR & TIMER_CR_EN && TIMER->LOAD) {
        w.timer_us_acc += 1000;
        while (w.timer_us_acc >= TIMER->LOAD) {
            w.timer_us_acc -= TIMER->LOAD;
            HW(TIMER->COUNT)++;
            HW(TIMER->SR) |= TIMER_SR_UIF;
            if (TIMER->CR & TIMER_CR_IE) raise_irq(TIMER_IRQHandler);
        }
    }

    // UART RX: deliver the next typed byte once the previous one has been acknowledged.
    if ((UART->CR & UART_CR_EN) && !(UART->SR & UART_SR_RXNE)) {
        pthread_mutex_lock(&rx_lock);
        bool have = rx_tail != rx_head;
        char c = have ? rx_pending[rx_tail] : 0;
        if (have) rx_tail = (rx_tail + 1) % sizeof rx_pending;
        pthread_mutex_unlock(&rx_lock);
        if (have) {
            HW(UART->RXDR) = (uint8_t)c;
            HW(UART->SR) |= UART_SR_RXNE;
            if (UART->CR & UART_CR_RXIE) raise_irq(UART_IRQHandler);
        }
    }

    // Fan: the PWM duty sets a target speed, and the rotor's inertia makes the real speed lag behind.
    double duty = (PWM->CR & PWM_CR_EN) && PWM->PERIOD ? (double)PWM->DUTY / PWM->PERIOD : 0.0;
    double target = (w.stalled || duty < 0.15) ? 0.0 : duty * FAN_MAX_RPM; // too little power: won't spin
    w.rpm += (target - w.rpm) * dt / 0.4;
    HW(PWM->TACH) = (uint32_t)(w.rpm + 0.5);

    // Thermal model: heat in, minus losses through the case and the fan's airflow.
    const double heat_capacity = 30.0, passive = 0.4, fan_max = 3.5; // J/K, W/K, W/K
    double cooling = (passive + fan_max * w.rpm / FAN_MAX_RPM) * (w.temp_c - w.ambient_c);
    w.temp_c += (w.heat_w - cooling) / heat_capacity * dt;
}

static void dashboard(void) {
    printf("sim  │ t=%5.1fs  temp %5.1f C  fan %4.0f rpm  duty %3u%%  LED %-3s heat %3.0f W%s\n", w.ms / 1000.0,
           w.temp_c, w.rpm, PWM->PERIOD ? (unsigned)(PWM->DUTY * 100 / PWM->PERIOD) : 0u,
           (GPIO->ODR >> GPIO_PIN_LED) & 1 ? "on" : "off", w.heat_w, w.stalled ? "  [FAN STALLED]" : "");
    fflush(stdout);
}

/* --------------------------------- scripts --------------------------------- */
typedef enum { EV_TYPE, EV_HEAT, EV_STALL, EV_REPAIR, EV_PRESS, EV_RELEASE, EV_END } EventKind;
typedef struct {
    uint32_t at_ms;
    EventKind kind;
    const char* text;
    double value;
} Event;

static const Event demo_script[] = {
    {1000, EV_TYPE, "help\n", 0},
    {3000, EV_TYPE, "status\n", 0},
    {5000, EV_HEAT, NULL, 60},
    {9000, EV_TYPE, "status\n", 0},
    {10000, EV_TYPE, "manual 30\n", 0},
    {13000, EV_TYPE, "auto\n", 0},
    {15000, EV_STALL, NULL, 0},
    {19000, EV_TYPE, "auto\n", 0},
    {20000, EV_REPAIR, NULL, 0},
    {21000, EV_PRESS, NULL, 0},
    {21300, EV_RELEASE, NULL, 0},
    {24000, EV_TYPE, "status\n", 0},
    {25000, EV_END, NULL, 0},
};
static size_t next_event;
static bool interactive;

static void sim_command(const char* cmd) {
    double v;
    if (sscanf(cmd, "heat %lf", &v) == 1) w.heat_w = v;
    else if (strcmp(cmd, "stall") == 0) w.stalled = true;
    else if (strcmp(cmd, "repair") == 0) w.stalled = false;
    else if (strcmp(cmd, "press") == 0) w.button_down = true;
    else if (strcmp(cmd, "release") == 0) w.button_down = false;
    else if (strcmp(cmd, "quit") == 0) atomic_store(&firmware_running, false);
    else {
        printf("sim  │ commands: !heat <watts> | !stall | !repair | !press | !release | !quit\n");
        return;
    }
    printf("sim  │ >>> %s\n", cmd);
}

static void run_script(void) {
    while (next_event < sizeof demo_script / sizeof demo_script[0] && demo_script[next_event].at_ms <= w.ms) {
        const Event* e = &demo_script[next_event++];
        switch (e->kind) {
        case EV_TYPE: printf("sim  │ >>> typing: %.*s\n", (int)strcspn(e->text, "\n"), e->text); type_text(e->text); break;
        case EV_HEAT: w.heat_w = e->value; printf("sim  │ >>> heat load is now %.0f W\n", e->value); break;
        case EV_STALL: w.stalled = true; printf("sim  │ >>> something jams the fan!\n"); break;
        case EV_REPAIR: w.stalled = false; printf("sim  │ >>> fan unjammed\n"); break;
        case EV_PRESS: w.button_down = true; printf("sim  │ >>> button pressed\n"); break;
        case EV_RELEASE: w.button_down = false; break;
        case EV_END: printf("sim  │ demo finished\n"); atomic_store(&firmware_running, false); break;
        }
    }
}

/* ------------------------------ threads & init ----------------------------- */
static long real_us_per_sim_ms = 100; // speed 10×

static void* hardware_thread(void* arg) {
    (void)arg;
    struct timespec last;
    clock_gettime(CLOCK_MONOTONIC, &last);
    for (;;) { // keeps running even after the demo ends, so the firmware can finish what it's sending
        service_fast();
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed = (now.tv_sec - last.tv_sec) * 1000000L + (now.tv_nsec - last.tv_nsec) / 1000;
        if (elapsed >= real_us_per_sim_ms) {
            last = now;
            service_tick();
            if (!interactive) run_script();
            if (w.ms % 2000 == 0) dashboard();
        } else {
            sleep_us(5);
        }
    }
    return NULL;
}

static void* stdin_thread(void* arg) {
    (void)arg;
    char line[128];
    while (fgets(line, sizeof line, stdin)) {
        if (line[0] == '!') {
            line[strcspn(line, "\n")] = '\0';
            sim_command(line + 1);
        } else {
            type_text(line);
        }
    }
    atomic_store(&firmware_running, false); // EOF (Ctrl-D)
    return NULL;
}

void hal_init(int argc, char** argv) {
    double speed = 10.0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) interactive = true, speed = 1.0;
        else if (strcmp(argv[i], "--speed") == 0 && i + 1 < argc) speed = atof(argv[++i]);
        else {
            fprintf(stderr, "usage: %s [-i] [--speed N]\n  -i  interactive: type firmware commands; !help for simulator commands\n", argv[0]);
            exit(2);
        }
    }
    if (speed <= 0) speed = 1;
    real_us_per_sim_ms = (long)(1000.0 / speed);
    if (real_us_per_sim_ms < 1) real_us_per_sim_ms = 1;
    setvbuf(stdout, NULL, _IOLBF, 0);
    printf("sim  │ FC-1 simulator: %s, %.0fx speed\n", interactive ? "interactive (type 'help' or '!help')" : "demo script", speed);

    pthread_t t;
    pthread_create(&t, NULL, hardware_thread, NULL);
    pthread_detach(t);
    if (interactive) {
        pthread_create(&t, NULL, stdin_thread, NULL);
        pthread_detach(t);
    }
}
