/* Linker script for the STM32F401RE (Nucleo-F401RE board).
 *
 * It tells the linker where memory is and where each section goes:
 *   - code and constants (.text, .rodata) → flash, which survives power-off
 *   - initialized globals (.data) → RAM at run time, with their initial values
 *     stored in flash. Reset_Handler copies them over (see startup.c).
 *   - zeroed globals (.bss) → RAM. Reset_Handler zero-fills it.
 * This is the "memory map" from chapter 22, made explicit. */
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* the stack starts at the top of RAM and grows down */

SECTIONS
{
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH   /* the CPU reads this table at address 0x08000000 */

    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    _sidata = LOADADDR(.data);          /* where .data's initial values are stored in flash */
    .data :
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    .bss (NOLOAD) :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /DISCARD/ : { *(.ARM.exidx*) *(.comment) }
}
