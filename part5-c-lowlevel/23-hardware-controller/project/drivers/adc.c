#include "adc.h"

#include "fc1.h"

void adc_init(void) { ADC->CR = ADC_CR_EN; }

bool adc_read(uint16_t* raw) {
    ADC->ICR = ADC_SR_EOC;     // clear a stale "done" flag (W1C)
    ADC->CMD = ADC_CMD_START;  // request a conversion (self-clearing)
    for (uint32_t spin = 0; spin < 5000000u; spin++) {
        if (ADC->SR & ADC_SR_EOC) { // volatile: re-read from hardware on every iteration
            *raw = (uint16_t)(ADC->DR & 0x0FFFu);
            ADC->ICR = ADC_SR_EOC;
            return true;
        }
    }
    return false;
}

int16_t adc_raw_to_celsius_x10(uint16_t raw) {
    // temp = raw * 125 / 4095 → ×10 for tenths; uint32_t intermediate avoids overflow.
    // Adding half the divisor rounds to nearest instead of truncating.
    return (int16_t)(((uint32_t)raw * ADC_MAX_TEMP_C * 10u + ADC_FULL_SCALE / 2) / ADC_FULL_SCALE);
}
