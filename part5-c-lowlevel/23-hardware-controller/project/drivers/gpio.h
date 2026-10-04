#ifndef GPIO_H
#define GPIO_H
#include <stdbool.h>

void gpio_init(void);
void gpio_led_set(bool on);
bool gpio_button_pressed(void);

#endif
