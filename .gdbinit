target remote :2331
file lcd.elf
monitor reset
monitor halt
monitor endian little
monitor speed auto
