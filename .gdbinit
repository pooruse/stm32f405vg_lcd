set print pretty on
set pagination off

#target remote 192.168.122.14:2331
target remote :2331
monitor reset
monitor halt
monitor speed auto
monitor flash device STM32F405VG
monitor flash download 1
monitor flash breakpoints 1
monitor endian little
