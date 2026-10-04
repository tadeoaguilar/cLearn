// fc1.h: register map of the FC-1, a small (imaginary) fan-controller MCU.
//
// This file plays the role of a vendor "device header" (like stm32f401xe.h from
// ST's CMSIS pack). It describes WHERE each peripheral lives in the address
// space and WHAT each bit means. It contains no code.
//
// Memory map (on real silicon, the peripherals sit at fixed physical addresses):
//
//   0x4000_0000  GPIO   general-purpose pins (LED on pin 0, button on pin 1)
//   0x4000_1000  ADC    12-bit analog-to-digital converter (temperature sensor)
//   0x4000_2000  PWM    fan speed output + tachometer input
//   0x4000_3000  UART   serial console
//   0x4000_4000  TIMER  periodic tick (like ARM's SysTick)
//
// Register access types used below (the datasheet would list one per field):
//   RW  read/write by software
//   RO  read-only: written by hardware (status)
//   WO  write-only: reads return 0
//   W1C "write 1 to clear": writing a 1 to a bit clears that flag; writing 0 does nothing
//   SC  "self-clearing": software writes 1 to request an action; hardware clears it when done
#ifndef FC1_H
#define FC1_H

#include <stddef.h>
#include <stdint.h>

// On the host, the "hardware" is a block of memory owned by the simulator.
// On a real chip, these would be plain constants like 0x40000000u.
#ifdef FC1_SIMULATOR
extern uint32_t fc1_sim_memory[];
#define FC1_PERIPH_BASE ((uintptr_t)fc1_sim_memory)
#else
#define FC1_PERIPH_BASE 0x40000000u
#endif

// `volatile` tells the compiler that every read and write has side effects it
// can't see: never cache a value in a register, never remove or reorder accesses.
// The __I / __O / __IO names follow ARM CMSIS conventions.
#define __I volatile const // read-only
#define __O volatile       // write-only
#define __IO volatile      // read/write

/* ---------------------------------- GPIO ---------------------------------- */
typedef struct {
    __IO uint32_t DIR;  // 0x00 RW  1 = output, 0 = input (one bit per pin)
    __I uint32_t IDR;   // 0x04 RO  input data: the current level of each pin
    __IO uint32_t ODR;  // 0x08 RW  output data
    __O uint32_t BSRR;  // 0x0C WO  bits 0-15: set pin, bits 16-31: reset pin (atomic, no read-modify-write)
} FC1_GPIO_Regs;

#define GPIO_PIN_LED 0u
#define GPIO_PIN_BUTTON 1u // active low: reads 0 while pressed

/* ---------------------------------- ADC ----------------------------------- */
typedef struct {
    __IO uint32_t CR;   // 0x00 RW  control
    __I uint32_t SR;    // 0x04 RO  status
    __I uint32_t DR;    // 0x08 RO  data: 12-bit result in bits 0-11
    __O uint32_t ICR;   // 0x0C W1C interrupt/flag clear
    __IO uint32_t CMD;  // 0x10 SC  commands
} FC1_ADC_Regs;

#define ADC_CR_EN (1u << 0)
#define ADC_SR_EOC (1u << 0) // end of conversion: DR holds a fresh value
#define ADC_CMD_START (1u << 0)
// Sensor transfer function (from the "datasheet"): temp_C = raw * 125 / 4095, i.e. 0..125 °C.
#define ADC_FULL_SCALE 4095u
#define ADC_MAX_TEMP_C 125u

/* ---------------------------------- PWM ----------------------------------- */
typedef struct {
    __IO uint32_t CR;     // 0x00 RW  control
    __IO uint32_t PERIOD; // 0x04 RW  counter period (ticks)
    __IO uint32_t DUTY;   // 0x08 RW  compare value: output is high for DUTY of PERIOD ticks
    __I uint32_t TACH;    // 0x0C RO  measured fan speed (RPM)
} FC1_PWM_Regs;

#define PWM_CR_EN (1u << 0)
#define FAN_MAX_RPM 3000u

/* ---------------------------------- UART ---------------------------------- */
typedef struct {
    __IO uint32_t CR;   // 0x00 RW  control
    __I uint32_t SR;    // 0x04 RO  status
    __I uint32_t RXDR;  // 0x08 RO  received byte (bits 0-7)
    __IO uint32_t TXDR; // 0x0C RW  byte to transmit (bits 0-7)
    __O uint32_t ICR;   // 0x10 W1C flag clear
    __IO uint32_t CMD;  // 0x14 SC  commands
} FC1_UART_Regs;

#define UART_CR_EN (1u << 0)
#define UART_CR_RXIE (1u << 1) // raise an interrupt when a byte arrives
#define UART_SR_RXNE (1u << 0) // receive register not empty
#define UART_SR_ORE (1u << 1)  // overrun: a byte arrived before the previous one was read
#define UART_CMD_SEND (1u << 0) // transmit TXDR; hardware clears this bit when it took the byte

/* ---------------------------------- TIMER --------------------------------- */
typedef struct {
    __IO uint32_t CR;   // 0x00 RW  control
    __I uint32_t SR;    // 0x04 RO  status
    __O uint32_t ICR;   // 0x08 W1C flag clear
    __IO uint32_t LOAD; // 0x0C RW  tick period in microseconds
    __I uint32_t COUNT; // 0x10 RO  ticks since enable
} FC1_TIMER_Regs;

#define TIMER_CR_EN (1u << 0)
#define TIMER_CR_IE (1u << 1)
#define TIMER_SR_UIF (1u << 0) // update (tick) flag

/* ------------------------------ instances -------------------------------- */
#define FC1_GPIO_BASE (FC1_PERIPH_BASE + 0x0000u)
#define FC1_ADC_BASE (FC1_PERIPH_BASE + 0x1000u)
#define FC1_PWM_BASE (FC1_PERIPH_BASE + 0x2000u)
#define FC1_UART_BASE (FC1_PERIPH_BASE + 0x3000u)
#define FC1_TIMER_BASE (FC1_PERIPH_BASE + 0x4000u)
#define FC1_PERIPH_SPAN 0x5000u

// The classic idiom: cast an integer address to a pointer to the register struct.
#define GPIO ((FC1_GPIO_Regs*)FC1_GPIO_BASE)
#define ADC ((FC1_ADC_Regs*)FC1_ADC_BASE)
#define PWM ((FC1_PWM_Regs*)FC1_PWM_BASE)
#define UART ((FC1_UART_Regs*)FC1_UART_BASE)
#define TIMER ((FC1_TIMER_Regs*)FC1_TIMER_BASE)

// Catch layout mistakes at compile time: offsets must match the datasheet exactly.
_Static_assert(offsetof(FC1_GPIO_Regs, BSRR) == 0x0C, "GPIO layout");
_Static_assert(offsetof(FC1_ADC_Regs, CMD) == 0x10, "ADC layout");
_Static_assert(offsetof(FC1_PWM_Regs, TACH) == 0x0C, "PWM layout");
_Static_assert(offsetof(FC1_UART_Regs, CMD) == 0x14, "UART layout");
_Static_assert(offsetof(FC1_TIMER_Regs, COUNT) == 0x10, "TIMER layout");

/* ------------------------------ interrupts -------------------------------- */
// On a Cortex-M these names would appear in the vector table (see port-stm32f401/startup.c).
void TIMER_IRQHandler(void);
void UART_IRQHandler(void);

#endif
