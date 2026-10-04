# Bare-metal build for the Nucleo-F401RE.
#
#   brew install --cask gcc-arm-embedded   # or: sudo apt install gcc-arm-none-eabi
#   brew install stlink                    # or: sudo apt install stlink-tools
#   make            # → firmware.elf, firmware.bin
#   make flash      # writes to the board over USB (ST-LINK)
#   make size       # how much flash/RAM is used
#   screen /dev/tty.usbmodem* 115200       # serial console (Ctrl-A K to quit)

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size
CFLAGS  = -mcpu=cortex-m4 -mthumb -std=c17 -O2 -g -Wall -Wextra -Wpedantic \
          -ffreestanding -ffunction-sections -fdata-sections
LDFLAGS = -T stm32f401re.ld -nostdlib -Wl,--gc-sections -Wl,-Map=firmware.map

firmware.bin: firmware.elf
	$(OBJCOPY) -O binary $< $@

firmware.elf: startup.c main.c stm32f401re.ld ../project/include/ringbuf.h
	$(CC) $(CFLAGS) $(LDFLAGS) startup.c main.c -lgcc -o $@

flash: firmware.bin
	st-flash --reset write firmware.bin 0x08000000

size: firmware.elf
	$(SIZE) $<

clean:
	rm -f firmware.elf firmware.bin firmware.map

.PHONY: flash size clean
