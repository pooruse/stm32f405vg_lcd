PROJECT:=lcd
ELF:=$(PROJECT).elf
BIN:=$(PROJECT).bin
HEX:=$(PROJECT).hex

INCLUDE_PATH +=-Iinc
INCLUDE_PATH +=-Irtt/inc
INCLUDE_PATH += -Ilib/CMSIS/Include
INCLUDE_PATH += -Icommon/inc
INCLUDE_PATH += -Ilib/STM32F4xx_StdPeriph_Driver/inc

PREFIX = arm-none-eabi-
CC := $(PREFIX)gcc
AS := $(PREFIX)ar
CXX := $(PREFIX)g++
OBJCOPY := $(PREFIX)objcopy

CFLAGS +=-DSTM32F40_41xxx

CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -D__FPU_PRESENT=1
CFLAGS += -D__FPU_USED=1
CFLAGS += -mthumb
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mfloat-abi=hard
CFLAGS += -mfpu=fpv4-sp-d16
CFLAGS += -ffunction-sections
#CFLAGS += -MD
CFLAGS += -fdata-sections
CFLAGS += -Wall
#CFLAGS += -Wextra
CFLAGS += -Wimplicit-function-declaration
#CFLAGS += -Wmissing-prototypes
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wredundant-decls
CFLAGS += -ggdb3
#CFLAGS += -gdwarf-2
CFLAGS += -g3

ifeq ($(MAKECMDGOALS), release)
  LDFILE=STM32F405VG_RELEASE.ld
  CFLAGS += -DEnable_IAP
  CFLAGS += -Os
else
  LDFILE=STM32F405VG.ld
  CFLAGS += -DDisable_IAP
  CFLAGS += -O0
endif

MAPFILE=$(PROJECT).map

LDFLAGS +=--static
LDFLAGS += -nostartfiles
LDFLAGS += -T$(LDFILE)
LDFLAGS += -Wl,-Map=$(MAPFILE)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -mthumb
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfpu=fpv4-sp-d16 
LDFLAGS += -mfloat-abi=hard
#LDFLAGS += -Wl,--start-group
#LDFLAGS += -lc
#LDFLAGS += -lgcc
#LDFLAGS += -lnosys
#LDFLAGS += -lm
#LDFLAGS += -Wl,--end-group
#LDFLAGS += -Wl,-u -Wl,_printf_float
STDLIB_SRC =lib/STM32F4xx_StdPeriph_Driver/src
STARTUP_SRC =common/src
APP_SRC =src
RTT_SRC=rtt/src

OBJ :=
SRC := $(shell find $(STDLIB_SRC) -name '*.c')
OBJ += $(addprefix ,$(SRC:%.c=%.o))
SRC := $(shell find $(STARTUP_SRC) -name '*.c')
OBJ += $(addprefix ,$(SRC:%.c=%.o))
SRC := $(shell find $(STARTUP_SRC) -name '*.s')
OBJ += $(addprefix ,$(SRC:%.s=%.o))
SRC := $(shell find $(APP_SRC) -name '*.c')
OBJ += $(addprefix ,$(SRC:%.c=%.o))
SRC := $(shell find $(RTT_SRC) -name '*.c')
OBJ += $(addprefix ,$(SRC:%.c=%.o))
OBJ := $(filter-out $(STDLIB_SRC)/stm32f4xx_fmc.o, $(OBJ))

MAIN_OBJ=main.o
MAIN_SRC=main.c

debug: $(ELF) $(HEX)
release: $(BIN)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJ) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(STDLIB_SRC)/%.o: $(STDLIB_SRC)/%.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(STARTUP_SRC)/%.o: $(STARTUP_SRC)/%.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(STARTUP_SRC)/%.o: $(STARTUP_SRC)/%.s
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(APP_SRC)/%.o: $(APP_SRC)/%.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(RTT_SRC)/%.o: $(RTT_SRC)/%.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

test_line.elf: test/test_line.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

test/test_line.o: test/test_line.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<


.PHONY: clean clean_src

clean_src:
	#-rm $(APP_SRC)/*.d
	-rm $(APP_SRC)/*.o
	-rm $(ELF)
	-rm $(HEX)
	-rm $(BIN)

clean:
	#-rm $(STDLIB_SRC)/*.d
	-rm $(STDLIB_SRC)/*.o
	#-rm $(STARTUP_SRC)/*.d
	-rm $(STARTUP_SRC)/*.o
	#-rm $(APP_SRC)/*.d
	-rm main.o
	-rm test/*.o
	-rm $(MAPFILE)
	-rm $(APP_SRC)/*.o
	-rm *.elf
	-rm *.hex
	-rm *.bin
