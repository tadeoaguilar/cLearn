# 23 — Writing a Hardware Controller

> Goal: write a device driver and the firmware around it. You will learn how C
> talks to hardware through memory-mapped registers, why `volatile` exists, how
> interrupts work and how to share data with them safely, and how to structure
> firmware so its logic can be tested on a PC.

You don't need a board. The main project runs against a **simulated
microcontroller**, and there is a real-hardware port for a ~$15 Nucleo board
when you're ready.

## 1. Memory-mapped I/O: hardware looks like memory

A microcontroller's peripherals (GPIO pins, timers, UARTs, ADCs) are
controlled by **registers** at fixed addresses. Writing to `0x40020014`
doesn't store a number in RAM: it drives voltages onto pins.

```
 address space of an STM32F401
 0xE000_0000 ┌───────────────────┐ Cortex-M core: SysTick, NVIC (interrupt controller)
 0x4000_0000 ├───────────────────┤ peripherals: GPIOA @ 0x4002_0000, USART2 @ 0x4000_4400, ...
 0x2000_0000 ├───────────────────┤ SRAM (96 KB): .data, .bss, heap, stack
 0x0800_0000 ├───────────────────┤ flash (512 KB): vector table, .text, .rodata
             └───────────────────┘
```

In C, you overlay a struct of `volatile uint32_t` on that address:

```c
typedef struct {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR;
} GPIO_TypeDef;
#define GPIOA ((GPIO_TypeDef*)0x40020000u)

GPIOA->BSRR = 1u << 5;   // a single store instruction: pin PA5 goes high, and the LED turns on
```

Vendors ship these definitions as "device headers" (ARM CMSIS). Our
imaginary chip's header is [`project/include/fc1.h`](project/include/fc1.h):
read it first.

## 2. `volatile`

Without `volatile` the optimizer assumes that memory changes only when
*your code* changes it, so:

```c
while (!(UART->SR & RXNE)) {}   // without volatile: read once, then loop forever
UART->DR = 'a'; UART->DR = 'b'; // without volatile: the first store is "dead" and gets removed
```

`volatile` means "every access is an observable side effect. Do each one,
exactly as written, in order." Use it for:
- memory-mapped registers
- variables shared between an interrupt handler and the main code
- `sig_atomic_t` flags set by signal handlers

It is **not** a threading primitive: it doesn't make `x++` atomic and it adds
no memory barriers. For threads use `<stdatomic.h>`. See `examples/02_volatile.c`.

## 3. Bits, masks and fields

| Operation | Idiom |
|-----------|-------|
| set bit n | `reg |= 1u << n;` |
| clear bit n | `reg &= ~(1u << n);` |
| toggle | `reg ^= 1u << n;` |
| test | `if (reg & (1u << n))` |
| write a field | `reg = (reg & ~(MASK << POS)) | (value << POS);` |
| read a field | `(reg >> POS) & MASK` |

Always use **unsigned** types (`1u`, `uint32_t`). Shifting a signed 1 into
bit 31 is undefined behaviour.

### Register access types

Datasheets label each field. Get these wrong and the driver misbehaves
in confusing ways:

| Type | Meaning | Driver code |
|------|---------|-------------|
| RW | normal read/write | read-modify-write to change some bits |
| RO | status set by hardware | only read it |
| WO | write-only (reads return 0) | **never** `|=`: that reads 0 and clobbers everything |
| W1C | write **1** to clear a flag | `REG->ICR = FLAG;` (not `|=`!) |
| SC | self-clearing: set it to start something, hardware clears it when done | write, then poll until it reads 0 |
| set/reset register (BSRR) | atomic per-pin set/clear | no read-modify-write race with interrupts |

## 4. Drivers and layers

```
 ┌──────────────────────────────────────────────┐
 │ app/main.c        superloop, console, events │
 │ app/controller.c  decisions: pure C, no I/O  │ ← unit-tested on the PC
 ├──────────────────────────────────────────────┤
 │ drivers/*.c       gpio adc pwm uart timer    │ ← the only code that touches registers
 ├──────────────────────────────────────────────┤
 │ include/fc1.h     register map               │
 ├──────────────────────────────────────────────┤
 │ hardware  ─── or ───  sim/fc1_sim.c          │
 └──────────────────────────────────────────────┘
```

A good driver:
- hides register details behind a small API (`adc_read`, `pwm_set_duty_percent`)
- never blocks forever: polling loops have a **timeout**
- uses integer math (small MCUs often lack an FPU) and checks ranges
- configures a peripheral while it's **disabled**, then enables it

## 5. Interrupts

Polling wastes time. Instead, a peripheral can raise an **interrupt**. The CPU
pauses `main`, saves its registers, jumps to the handler listed in the
**vector table**, and resumes `main` afterwards.

```
 main loop ──────────┐        ┌──────────── main continues (unaware)
                     ▼        │
            UART_IRQHandler: read byte → push to queue → ack flag → return
```

Rules for interrupt handlers (ISRs):
1. **Short and fast.** No `printf`, no `malloc`, no blocking waits.
2. **Acknowledge** the source (clear the flag), or the handler re-fires forever.
3. Shared variables are `volatile`. Data **bigger than one word** needs a
   critical section (`irq_disable()`/`irq_enable()`, which is `cpsid i`/`cpsie i`
   on ARM) or a lock-free structure.
4. To pass a stream of data, use a **single-producer/single-consumer ring
   buffer** ([`ringbuf.h`](project/include/ringbuf.h)). The ISR only writes
   `head` and `main` only writes `tail`, so no lock is needed.

`examples/04_interrupt_shared_data.c` shows a real torn read happening, and
two ways to fix it.

## 6. The project: FC-1 fan controller

The firmware reads a temperature sensor (ADC), drives a fan (PWM), watches the
fan's tachometer, blinks a status LED, reads a push button, and offers a
serial console:

- **AUTO**: a fan curve with **hysteresis** (on at 32 °C, off below 28 °C, linear up to 100 % at 60 °C)
- **MANUAL**: set the duty from the console (`manual 40`)
- **FAULT**: the fan is driven but not spinning for 2 s (stall), or the
  temperature is above 85 °C. The firmware forces 100 % (fail-safe), raises an
  alarm, blinks the LED fast, and waits for a human to press the button.

```bash
# with CMake (from the repo root)
cmake -S . -B build && cmake --build build -j
./build/part5-c-lowlevel/23-hardware-controller/ch23_fan_controller        # scripted 25 s demo, at 10× speed
./build/part5-c-lowlevel/23-hardware-controller/ch23_fan_controller -i     # interactive, real time
ctest --test-dir build -R ch23

# or by hand, from this folder
cc -std=c17 -Wall -Wextra -DFC1_SIMULATOR -Iproject/include -Iproject/drivers -Iproject/app \
   project/app/*.c project/drivers/*.c project/sim/fc1_sim.c -pthread -o fan && ./fan
```

In interactive mode, type firmware commands (`help`, `status`, `manual 50`,
`auto`) or simulator commands that change the physical world: `!heat 70`,
`!stall`, `!repair`, `!press`, `!release`, `!quit`.

Sample of the demo output:

```
sim  │ >>> heat load is now 60 W
sim  │ t=  8.0s  temp  33.1 C  fan  735 rpm  duty  32%  LED on  heat  60 W
sim  │ >>> something jams the fan!
sim  │ t= 18.0s  temp  46.7 C  fan    1 rpm  duty  67%  LED on  heat  60 W  [FAN STALLED]
uart │ ALARM: fan stall! fan forced to 100%. Press the button to acknowledge.
sim  │ >>> button pressed
uart │ fault acknowledged, back to AUTO
```

### How the simulator works

[`sim/fc1_sim.c`](project/sim/fc1_sim.c) is the "silicon". It owns the
memory that `fc1.h` points the peripherals at, and a background thread
behaves like hardware:
- it notices command bits (ADC `START`, UART `SEND`), performs the action and clears them
- it sets status flags and applies W1C clears
- it calls `TIMER_IRQHandler` every simulated millisecond and `UART_IRQHandler`
  when a byte arrives, while holding the lock that `irq_disable()` takes. So
  "interrupts disabled" really blocks them.
- it simulates physics: heat flow, fan inertia, sensor noise, and a jammed fan

The drivers don't know they're simulated. `#ifdef FC1_SIMULATOR` appears
only in `fc1.h` (the base address) and `hal.h` (sleep and interrupt masking).

## 7. On real hardware: `port-stm32f401/`

[`port-stm32f401/`](port-stm32f401/) is a complete bare-metal program for the
**Nucleo-F401RE** with no vendor library: LED blink, button, UART console with
an RX interrupt, and a SysTick timer. It shows the pieces that the
operating system normally provides:

- **`stm32f401re.ld`**, the linker script: where flash and RAM are, and where each section goes
- **`startup.c`**, the vector table and `Reset_Handler`. It copies `.data` from flash to RAM,
  zeroes `.bss` (this is why globals start at 0!) and then calls `main`.
- **`main.c`**: clock enables, pin multiplexing, the baud rate divisor, NVIC enable

```bash
cd port-stm32f401 && make && make flash
screen /dev/tty.usbmodem* 115200
```

> This port has been cross-compiled warning-free with `clang
> --target=thumbv7em-none-eabi`, but it has **not** been linked with
> `arm-none-eabi-gcc` or run on a physical board yet. The register addresses
> come from RM0368. If it doesn't blink, debugging it is half the education:
> check the clock enable bits first.

Porting the FC-1 project to the board (exercise 6) means rewriting only
`fc1.h`'s addresses and the five drivers: TIM2 for PWM, TIM3 input capture for
the tachometer, ADC1, USART2 and SysTick. `controller.c` doesn't change at all.

## Examples

| File | Shows |
|------|-------|
| `01_bit_manipulation.c` | set/clear/toggle/test, multi-bit fields, endianness, popcount/ctz |
| `02_volatile.c` | polling a status bit that "hardware" (another thread) sets |
| `03_register_map.c` | an STM32 GPIO block as a struct, `offsetof` checks, why not to use bit-fields |
| `04_interrupt_shared_data.c` | torn reads of multi-word data; critical section and retry-loop fixes |
| `05_state_machine.c` | a table-driven finite state machine (a garage door) |

## Pitfalls

- `REG |= FLAG` on a W1C register clears **every** pending flag (you write back the 1s you just read).
- Forgetting to enable a peripheral's clock: the registers read as 0 and writes are ignored.
- Doing work in an ISR that belongs in the main loop.
- Polling without a timeout: one dead sensor hangs the entire device.
- Doing `a + b` with `uint8_t` values and expecting wrap-around (they're promoted to `int`).
- Timing with `now > deadline` instead of `now - start >= interval`: the
  first form breaks when the 32-bit counter wraps (after ~49 days at 1 kHz).

➡️ Next: [chapter 24](../24-c-and-asm/README.md), the instructions underneath all this C.
