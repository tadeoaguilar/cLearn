// Unit tests for the controller logic. They run on the PC with no hardware or simulator:
// that is the payoff of keeping controller.c free of register accesses.
#include <stdio.h>

#include "controller.h"

static int failures, checks;
#define CHECK(cond)                                                                                                    \
    do {                                                                                                               \
        checks++;                                                                                                      \
        if (!(cond)) {                                                                                                 \
            failures++;                                                                                                \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                                                     \
        }                                                                                                              \
    } while (0)

static void test_fan_curve_hysteresis(void) {
    CtlConfig cfg = ctl_default_config();
    bool running = false;
    CHECK(ctl_fan_curve(&cfg, 250, &running) == 0 && !running);
    CHECK(ctl_fan_curve(&cfg, 310, &running) == 0 && !running); // between off and on: stays off
    CHECK(ctl_fan_curve(&cfg, 320, &running) > 0 && running);   // reaches the on threshold
    CHECK(ctl_fan_curve(&cfg, 300, &running) >= cfg.min_duty);  // cooling down: still on (hysteresis)
    CHECK(ctl_fan_curve(&cfg, 279, &running) == 0 && !running); // below the off threshold
    running = true;
    CHECK(ctl_fan_curve(&cfg, 600, &running) == 100);
    CHECK(ctl_fan_curve(&cfg, 900, &running) == 100);
    uint8_t mid = ctl_fan_curve(&cfg, 440, &running); // halfway between 28 and 60 °C
    CHECK(mid == 60);
}

static void test_curve_is_monotonic(void) {
    CtlConfig cfg = ctl_default_config();
    bool running = true;
    uint8_t prev = 0;
    for (int16_t t = 280; t <= 700; t += 5) {
        uint8_t d = ctl_fan_curve(&cfg, t, &running);
        CHECK(d >= prev);
        prev = d;
    }
}

static void test_stall_detection_and_ack(void) {
    CtlConfig cfg = ctl_default_config();
    Controller c;
    ctl_init(&c, &cfg);
    CtlInputs in = {.temp_c10 = 500, .rpm = 1500};
    uint32_t t = 0;
    for (int i = 0; i < 10; i++) ctl_step(&c, &in, t += 100, 100);
    CHECK(c.mode == MODE_AUTO);

    in.rpm = 0; // the fan stops while it is being driven
    for (int i = 0; i < 19; i++) ctl_step(&c, &in, t += 100, 100);
    CHECK(c.mode == MODE_AUTO); // 1.9 s: not yet
    CtlOutputs out = ctl_step(&c, &in, t += 100, 100);
    CHECK(c.mode == MODE_FAULT && c.fault == FAULT_FAN_STALL);
    CHECK(out.duty == 100);

    CHECK(!ctl_set_auto(&c));        // the console can't clear a fault
    CHECK(!ctl_set_manual(&c, 50));

    in.rpm = 2500;
    in.button = true; // press → acknowledge
    ctl_step(&c, &in, t += 100, 100);
    CHECK(c.mode == MODE_AUTO && c.fault == FAULT_NONE);
    ctl_step(&c, &in, t += 100, 100); // holding the button must not do anything else
    CHECK(c.mode == MODE_AUTO);
}

static void test_overtemp(void) {
    CtlConfig cfg = ctl_default_config();
    Controller c;
    ctl_init(&c, &cfg);
    CHECK(ctl_set_manual(&c, 10));
    CtlInputs in = {.temp_c10 = 900, .rpm = 0};
    CtlOutputs out = ctl_step(&c, &in, 100, 100);
    CHECK(c.mode == MODE_FAULT && c.fault == FAULT_OVERTEMP && out.duty == 100);
}

static void test_manual_mode_and_led(void) {
    CtlConfig cfg = ctl_default_config();
    Controller c;
    ctl_init(&c, &cfg);
    CHECK(!ctl_set_manual(&c, 101));
    CHECK(ctl_set_manual(&c, 40));
    CtlInputs in = {.temp_c10 = 250, .rpm = 1200};
    CtlOutputs a = ctl_step(&c, &in, 100, 100);
    CtlOutputs b = ctl_step(&c, &in, 600, 500);
    CHECK(a.duty == 40 && b.duty == 40);
    CHECK(a.led != b.led); // 1 Hz blink: 500 ms apart → opposite phase
}

int main(void) {
    test_fan_curve_hysteresis();
    test_curve_is_monotonic();
    test_stall_detection_and_ack();
    test_overtemp();
    test_manual_mode_and_led();
    printf("%d/%d checks passed\n", checks - failures, checks);
    return failures ? 1 : 0;
}
