#ifndef ADC_H
#define ADC_H
#include <stdbool.h>
#include <stdint.h>

void adc_init(void);
// Starts a conversion and polls until it finishes. Returns false on timeout
// (a dead sensor must never hang the whole firmware).
bool adc_read(uint16_t* raw);
// Converts a raw reading to tenths of a degree: 412 means 41.2 °C.
// Integer math only, since small MCUs often have no floating-point unit.
int16_t adc_raw_to_celsius_x10(uint16_t raw);

#endif
