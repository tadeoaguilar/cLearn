#ifndef PWM_H
#define PWM_H
#include <stdint.h>

void pwm_init(uint32_t period_ticks);
void pwm_set_duty_percent(uint8_t percent); // clamped to 0..100
uint16_t pwm_fan_rpm(void);

#endif
