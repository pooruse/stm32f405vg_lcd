# stm32f405vg_lcd

this project use arm-none-eabi-gcc compiler
drive st7929 lcd screen in serial mode

I have done the spi 1-way bidirection mode design,
but it seens can't support read busy flag,
so I wait busy by delay loop.

