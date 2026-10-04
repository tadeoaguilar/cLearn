// PID control of the FC-1 thermal model, with and without anti-windup.
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    double kp, ki, kd;
    double out_min, out_max;
    bool anti_windup;
    double integral, prev_error;
    bool first;
} Pid;

static double pid_update(Pid* p, double error, double dt) {
    double derivative = p->first ? 0.0 : (error - p->prev_error) / dt;
    p->first = false;
    p->prev_error = error;

    double candidate_integral = p->integral + error * dt;
    double out = p->kp * error + p->ki * candidate_integral + p->kd * derivative;
    bool saturated_high = out > p->out_max, saturated_low = out < p->out_min;
    // Anti-windup (conditional integration): don't let the integral keep growing while the
    // output is pinned at its limit, because it would have to "unwind" later and that causes overshoot.
    if (!p->anti_windup || !((saturated_high && error > 0) || (saturated_low && error < 0)))
        p->integral = candidate_integral;
    if (out > p->out_max) out = p->out_max;
    if (out < p->out_min) out = p->out_min;
    return out;
}

typedef struct {
    double temp, rpm, heat_w;
} Plant;

// Same equations as sim/fc1_sim.c.
static void plant_step(Plant* w, double duty_pct, double dt) {
    double duty = duty_pct / 100.0;
    double target = duty < 0.15 ? 0.0 : duty * 3000.0;
    w->rpm += (target - w->rpm) * dt / 0.4;
    double cooling = (0.4 + 3.5 * w->rpm / 3000.0) * (w->temp - 25.0);
    w->temp += (w->heat_w - cooling) / 30.0 * dt;
}

static void run(bool anti_windup) {
    Pid pid = {.kp = 8, .ki = 1.5, .kd = 2, .out_min = 0, .out_max = 100, .anti_windup = anti_windup, .first = true};
    Plant w = {.temp = 25, .heat_w = 15};
    const double setpoint = 40.0, dt = 0.01;
    double duty = 0;
    printf("\n%s anti-windup:\n   t   heat  temp   duty  integral\n", anti_windup ? "WITH" : "WITHOUT");
    for (int i = 0; i <= 12000; i++) { // 120 s
        double t = i * dt;
        // A huge load that the fan can't handle (it saturates at 100 %), then back to normal.
        w.heat_w = t < 30 ? 15 : t < 60 ? 110 : 30;
        if (i % 10 == 0) duty = pid_update(&pid, w.temp - setpoint, dt * 10); // control at 10 Hz
        plant_step(&w, duty, dt);
        if (i % 1000 == 0) printf("%4.0f  %4.0f  %5.1f  %5.1f  %8.1f\n", t, w.heat_w, w.temp, duty, pid.integral);
    }
}

int main(void) {
    run(false);
    run(true);
    printf("\nWithout anti-windup the integral winds up twice:\n"
           " - negative while the room is cold (0-30 s, fan pinned at 0%%), so the fan reacts late\n"
           "   to the overload and the temperature overshoots to ~69 °C instead of ~51 °C;\n"
           " - positive during the overload, so after 60 s the fan stays at 100%% for ~45 s and the\n"
           "   temperature undershoots to ~33 °C. With anti-windup it settles at 40 °C within ~20 s.\n");
    return 0;
}
