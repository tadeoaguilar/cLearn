#include "gpio.h"

#include "fc1.h"

void gpio_init(void) {
    GPIO->DIR |= 1u << GPIO_PIN_LED;     // LED pin: output
    GPIO->DIR &= ~(1u << GPIO_PIN_BUTTON); // button pin: input
    gpio_led_set(false);
}

void gpio_led_set(bool on) {
    // BSRR changes one pin with a single write. `GPIO->ODR |= bit` would be a
    // read-modify-write: if an interrupt changed another ODR bit between the read
    // and the write, that change would be lost.
    GPIO->BSRR = on ? (1u << GPIO_PIN_LED) : (1u << (GPIO_PIN_LED + 16));
}

bool gpio_button_pressed(void) {
    return (GPIO->IDR & (1u << GPIO_PIN_BUTTON)) == 0; // active low
}
