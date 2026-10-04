# 23 — Exercises: Hardware Controller

Exercises 1–5 are single files that run on your PC. Exercise 6 needs a board.

### Ex 1 — A register-field library ⭐
Write `static inline` functions (not macros) for a 32-bit register:
`reg_set_bits`, `reg_clear_bits`, `reg_write_field(reg, pos, width, value)`
and `reg_read_field`, all taking a `volatile uint32_t*`. `reg_write_field`
must reject (return `false`) a value that doesn't fit in `width` bits.
Test them on an STM32-style `MODER` (2 bits per pin) and `AFR` (4 bits per pin).
Then write a fake W1C status register and show **why** `ICR |= FLAG_A` is a
bug when FLAG_B is also pending.
→ `solutions/ex01_register_fields.c`

### Ex 2 — Button debouncer ⭐⭐
A mechanical button bounces for ~5–20 ms. Write an **integrator** debouncer
that is called every 1 ms with the raw pin level. It counts up (max N) while
the pin reads pressed and down (min 0) while released, and changes state
only on reaching N or 0. Report a press **event** once per press (edge
detection). Feed it a generated noisy signal (bounces at both edges plus a
1 ms glitch in the middle) and show exactly 2 presses are detected.
→ `solutions/ex02_debounce.c`

### Ex 3 — Filtering sensor noise with integer math ⭐⭐
ADC readings are noisy and sometimes contain spikes. Implement:
- a **moving average** of the last 8 samples using a ring buffer and a running sum (O(1) per sample, no floats)
- a **median of 5** filter (removes single spikes; averages don't)
- an **exponential filter** `y += (x - y) >> 3` in fixed point

Run all three over the same signal (a slow ramp + noise + two spikes) and print a comparison table.
→ `solutions/ex03_adc_filters.c`

### Ex 4 — A console command interpreter ⭐⭐
Replace the `if/else strcmp` chain in `project/app/main.c` with a table:
`{ name, min_args, max_args, handler, help }`. Handlers receive `argc/argv`
(split in place, no heap). Unknown commands and wrong argument counts produce
a helpful error, and `help` is generated from the table. Test it with a scripted list of lines.
→ `solutions/ex04_command_table.c`

### Ex 5 — PID temperature control ⭐⭐⭐
Instead of a fixed fan curve, hold the temperature at a **setpoint** with a
PID controller:
`duty = Kp·e + Ki·∫e dt + Kd·de/dt`, where `e = temp − setpoint` (the fan cools, so positive error → more fan).
Use the same thermal model as `sim/fc1_sim.c` (copy the equations). Implement
output clamping to 0..100 and **anti-windup**: stop integrating while the
output is saturated. Compare with no anti-windup after a big heat step and
print the overshoot. Bonus: add a `pid` mode to the real project.
→ `solutions/ex05_pid.c`

### Ex 6 — Port the FC-1 to real silicon ⭐⭐⭐⭐ (open project, no solution)
With a Nucleo-F401RE, a 4-pin PC fan, a 12 V supply, a MOSFET (or simply the
fan's PWM pin, which accepts 3.3 V logic) and an NTC thermistor or a TMP36:
- map the drivers: PWM → TIM2 CH1 at 25 kHz (the PC fan standard), tachometer → TIM3 input
  capture (2 pulses per revolution), temperature → ADC1, console → USART2, tick → SysTick
- start from `port-stm32f401/` (startup, linker script, UART) and keep `controller.c` unchanged
- the only `#ifdef`s you should need are in `hal.h`
