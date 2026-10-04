// The firmware: initialize the peripherals, then run the "superloop" forever.
//
// Every 100 ms (from the timer interrupt's millisecond counter) it samples the
// temperature and fan speed, runs the controller and updates the PWM and LED.
// Between those control steps it serves the serial console and sleeps until the
// next interrupt.
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "controller.h"
#include "gpio.h"
#include "hal.h"
#include "pwm.h"
#include "timer.h"
#include "uart.h"

enum { CONTROL_PERIOD_MS = 100, DEBOUNCE_SAMPLES = 3 };

static Controller ctl;
static int16_t last_temp_c10;
static uint16_t last_rpm;

static void print_status(void) {
    uart_printf("mode=%s fault=%s temp=%d.%d C fan=%u rpm duty=%u%% rx_dropped=%lu\n", ctl_mode_name(ctl.mode),
                ctl_fault_name(ctl.fault), last_temp_c10 / 10, last_temp_c10 % 10, (unsigned)last_rpm,
                (unsigned)ctl.duty, (unsigned long)uart_rx_dropped());
}

static void handle_command(char* line) {
    char* cmd = strtok(line, " ");
    char* arg = strtok(NULL, " ");
    if (!cmd) return;
    if (strcmp(cmd, "status") == 0) {
        print_status();
    } else if (strcmp(cmd, "auto") == 0) {
        uart_puts(ctl_set_auto(&ctl) ? "ok: automatic mode\n" : "error: press the button to clear the fault\n");
    } else if (strcmp(cmd, "manual") == 0 && arg) {
        char* end;
        long duty = strtol(arg, &end, 10);
        bool ok = *end == '\0' && duty >= 0 && duty <= 100 && ctl_set_manual(&ctl, (uint8_t)duty);
        if (ok) uart_printf("ok: manual %ld%%\n", duty);
        else uart_puts("error: usage 'manual <0-100>', not allowed during a fault\n");
    } else if (strcmp(cmd, "help") == 0) {
        uart_puts("commands: status | auto | manual <0-100> | help\n");
    } else {
        uart_printf("error: unknown command '%s' (try help)\n", cmd);
    }
}

// Collects characters into a line; runs the command on Enter.
static void console_poll(void) {
    static char line[48];
    static size_t len;
    char c;
    while (uart_getc(&c)) {
        if (c == '\r' || c == '\n') {
            line[len] = '\0';
            if (len > 0) handle_command(line);
            len = 0;
        } else if (len < sizeof line - 1) {
            line[len++] = c;
        }
    }
}

// A switch "bounces": it flips several times within a few ms when pressed.
// Accept a new state only after it has been stable for several samples.
static bool debounce_button(bool raw) {
    static bool stable;
    static uint8_t count;
    if (raw == stable) count = 0;
    else if (++count >= DEBOUNCE_SAMPLES) {
        stable = raw;
        count = 0;
    }
    return stable;
}

int main(int argc, char** argv) {
    hal_init(argc, argv);
    gpio_init();
    adc_init();
    pwm_init(1000);
    uart_init();
    timer_init(1000); // 1 ms tick

    CtlConfig cfg = ctl_default_config();
    ctl_init(&ctl, &cfg);
    uart_puts("FC-1 fan controller v1.0 ready. Type 'help'.\n");

    uint32_t last_step = millis();
    CtlMode last_mode = ctl.mode;
    while (hal_running()) {
        uint32_t now = millis();
        if (now - last_step >= CONTROL_PERIOD_MS) { // subtraction is wrap-around safe
            uint32_t dt = now - last_step;
            last_step = now;

            uint16_t raw;
            if (adc_read(&raw)) last_temp_c10 = adc_raw_to_celsius_x10(raw);
            last_rpm = pwm_fan_rpm();
            CtlInputs in = {.temp_c10 = last_temp_c10, .rpm = last_rpm, .button = debounce_button(gpio_button_pressed())};
            CtlOutputs out = ctl_step(&ctl, &in, now, dt);
            pwm_set_duty_percent(out.duty);
            gpio_led_set(out.led);

            if (ctl.mode != last_mode) { // report state changes, the way a real device logs events
                if (ctl.mode == MODE_FAULT) uart_printf("ALARM: %s! fan forced to 100%%. Press the button to acknowledge.\n", ctl_fault_name(ctl.fault));
                else if (last_mode == MODE_FAULT) uart_puts("fault acknowledged, back to AUTO\n");
                last_mode = ctl.mode;
            }
        }
        console_poll();
        wait_for_interrupt(); // sleep: saves power, and the timer wakes us every ms
    }
    return 0;
}
