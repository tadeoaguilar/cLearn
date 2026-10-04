#include "pwm.h"

#include "fc1.h"

void pwm_init(uint32_t period_ticks) {
    PWM->CR = 0;               // configure while disabled: avoids glitches on the output pin
    PWM->PERIOD = period_ticks;
    PWM->DUTY = 0;
    PWM->CR = PWM_CR_EN;
}

void pwm_set_duty_percent(uint8_t percent) {
    if (percent > 100) percent = 100;
    PWM->DUTY = PWM->PERIOD * percent / 100u;
}

uint16_t pwm_fan_rpm(void) { return (uint16_t)PWM->TACH; }
