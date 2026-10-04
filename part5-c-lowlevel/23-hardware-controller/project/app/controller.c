#include "controller.h"

CtlConfig ctl_default_config(void) {
    return (CtlConfig){
        .fan_on_c10 = 320,
        .fan_off_c10 = 280,
        .full_speed_c10 = 600,
        .min_duty = 20,
        .overtemp_c10 = 850,
        .stall_rpm = 100,
        .stall_ms = 2000,
    };
}

void ctl_init(Controller* c, const CtlConfig* cfg) {
    *c = (Controller){.cfg = *cfg, .mode = MODE_AUTO, .fault = FAULT_NONE};
}

uint8_t ctl_fan_curve(const CtlConfig* cfg, int16_t t, bool* running) {
    // Hysteresis: without it, a temperature hovering at 30.0 °C would switch the
    // fan on and off every few milliseconds ("chattering").
    if (!*running && t >= cfg->fan_on_c10) *running = true;
    if (*running && t < cfg->fan_off_c10) *running = false;
    if (!*running) return 0;
    if (t >= cfg->full_speed_c10) return 100;
    if (t <= cfg->fan_off_c10) return cfg->min_duty;
    // Linear from min_duty at fan_off to 100 % at full_speed (integer math, rounded).
    int32_t span_t = cfg->full_speed_c10 - cfg->fan_off_c10;
    int32_t span_d = 100 - cfg->min_duty;
    return (uint8_t)(cfg->min_duty + ((t - cfg->fan_off_c10) * span_d + span_t / 2) / span_t);
}

static void enter_fault(Controller* c, CtlFault f) {
    c->mode = MODE_FAULT;
    c->fault = f;
}

CtlOutputs ctl_step(Controller* c, const CtlInputs* in, uint32_t now_ms, uint32_t dt_ms) {
    bool button_pressed_now = in->button && !c->prev_button; // edge, not level
    c->prev_button = in->button;

    // Safety checks come first and apply in every mode.
    if (in->temp_c10 >= c->cfg.overtemp_c10 && c->mode != MODE_FAULT) enter_fault(c, FAULT_OVERTEMP);

    if (c->duty >= c->cfg.min_duty && in->rpm < c->cfg.stall_rpm) {
        c->stalled_for_ms += dt_ms;
        if (c->stalled_for_ms >= c->cfg.stall_ms && c->mode != MODE_FAULT) enter_fault(c, FAULT_FAN_STALL);
    } else {
        c->stalled_for_ms = 0;
    }

    switch (c->mode) {
    case MODE_AUTO:
        c->duty = ctl_fan_curve(&c->cfg, in->temp_c10, &c->fan_running);
        break;
    case MODE_MANUAL:
        c->duty = c->manual_duty;
        break;
    case MODE_FAULT:
        c->duty = 100; // fail safe: maximum cooling while a human looks at it
        if (button_pressed_now) { // acknowledge: retry automatic mode
            c->mode = MODE_AUTO;
            c->fault = FAULT_NONE;
            c->stalled_for_ms = 0;
        }
        break;
    }

    CtlOutputs out = {.duty = c->duty};
    switch (c->mode) {
    case MODE_AUTO: out.led = c->duty > 0; break;              // solid while the fan runs
    case MODE_MANUAL: out.led = (now_ms / 500) % 2; break;     // 1 Hz blink
    case MODE_FAULT: out.led = (now_ms / 100) % 2; break;      // 5 Hz blink: alarm
    }
    return out;
}

bool ctl_set_auto(Controller* c) {
    if (c->mode == MODE_FAULT) return false; // only the button clears a fault
    c->mode = MODE_AUTO;
    return true;
}

bool ctl_set_manual(Controller* c, uint8_t duty) {
    if (c->mode == MODE_FAULT || duty > 100) return false;
    c->mode = MODE_MANUAL;
    c->manual_duty = duty;
    return true;
}

const char* ctl_mode_name(CtlMode m) {
    static const char* const names[] = {"AUTO", "MANUAL", "FAULT"};
    return names[m];
}

const char* ctl_fault_name(CtlFault f) {
    static const char* const names[] = {"none", "fan stall", "over-temperature"};
    return names[f];
}
