// controller.h: the fan controller's decision logic.
//
// This module never touches a register. It takes sensor readings in and returns
// actuator commands, so it can be unit-tested on a PC (tests/test_controller.c)
// and reused unchanged on any chip. Keeping logic separate from I/O is the most
// important design rule in embedded software.
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { MODE_AUTO, MODE_MANUAL, MODE_FAULT } CtlMode;
typedef enum { FAULT_NONE, FAULT_FAN_STALL, FAULT_OVERTEMP } CtlFault;

// Tunables. They live in a struct, not #defines, so tests can change them.
typedef struct {
    int16_t fan_on_c10;     // fan starts above this temperature (tenths of °C)
    int16_t fan_off_c10;    // ...and stops below this one. The gap between them is the hysteresis.
    int16_t full_speed_c10; // 100 % duty at or above this
    uint8_t min_duty;       // the slowest duty at which the fan reliably spins
    int16_t overtemp_c10;   // critical temperature → fault
    uint16_t stall_rpm;     // below this the fan counts as stopped
    uint32_t stall_ms;      // how long it may stay stopped while driven
} CtlConfig;

typedef struct {
    int16_t temp_c10;
    uint16_t rpm;
    bool button; // debounced: true = pressed
} CtlInputs;

typedef struct {
    uint8_t duty;
    bool led;
} CtlOutputs;

typedef struct {
    CtlConfig cfg;
    CtlMode mode;
    CtlFault fault;
    uint8_t duty;
    uint8_t manual_duty;
    bool fan_running;         // hysteresis state
    uint32_t stalled_for_ms;  // time the fan has been driven but not spinning
    bool prev_button;
} Controller;

CtlConfig ctl_default_config(void);
void ctl_init(Controller* c, const CtlConfig* cfg);

// Called periodically with the time since the previous call.
CtlOutputs ctl_step(Controller* c, const CtlInputs* in, uint32_t now_ms, uint32_t dt_ms);

// Console commands. They return false if the command isn't allowed right now.
bool ctl_set_auto(Controller* c);
bool ctl_set_manual(Controller* c, uint8_t duty);

// Pure helper, exposed for testing: the duty for a temperature, with hysteresis.
uint8_t ctl_fan_curve(const CtlConfig* cfg, int16_t temp_c10, bool* running);

const char* ctl_mode_name(CtlMode m);
const char* ctl_fault_name(CtlFault f);

#endif
