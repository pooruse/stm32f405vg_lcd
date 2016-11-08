PROJECT:=lcd
ELF:=$(PROJECT).elf
BIN:=$(PROJECT).bin
HEX:=$(PROJECT).hex

INCLUDE_PATH +=-Iinc
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
#CFLAGS += -Os
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb
CFLAGS += -ffunction-sections
CFLAGS += -MD
CFLAGS += -fdata-sections
CFLAGS += -Wall
#CFLAGS += -Wextra
CFLAGS += -Wimplicit-function-declaration
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wredundant-decls
CFLAGS += -ggdb3 

LDFILE=STM32F405VG.ld
MAPFILE=$(PROJECT).map

LDFLAGS +=--static
LDFLAGS += -nostartfiles
LDFLAGS += -T$(LDFILE)
LDFLAGS += -Wl,-Map=$(MAPFILE)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -mthumb
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -Wl,--start-group
LDFLAGS += -lc
LDFLAGS += -lgcc
LDFLAGS += -lnosys
LDFLAGS += -Wl,--end-group

STDLIB_SRC =lib/STM32F4xx_StdPeriph_Driver/src
STARTUP_SRC =common/src
APP_SRC =src

OBJ :=

SRC := $(shell find $(STDLIB_SRC) -name '*.c')
OBJ += $(addprefix ,$(SRC:%.c=%.o))
SRC := $(shell find $(STARTUP_SRC) -name '*.c')
OBJ += $(addprefix ,$(SRC:%.c=%.o))
SRC := $(shell find $(STARTUP_SRC) -name '*.s')
OBJ += $(addprefix ,$(SRC:%.s=%.o))
SRC := $(shell find $(APP_SRC) -name '*.c')
OBJ += $(addprefix ,$(SRC:%.c=%.o))

OBJ := $(filter-out $(STDLIB_SRC)/stm32f4xx_fmc.o, $(OBJ))

all: $(BIN) $(HEX)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(STDLIB_SRC)/%.o: $(STDLIB_SRC)/%.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(STARTUP_SRC)/%.o: $(STARTUP_SRC)/%.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(STARTUP_SRC)/%.o: $(STARTUP_SRC)/%.s
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

$(APP_SRC)/%.o: $(APP_SRC)/%.c
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDE_PATH) $<

.PHONY: clean

clean:
	-rm $(STDLIB_SRC)/*.d
	-rm $(STDLIB_SRC)/*.o
	-rm $(STARTUP_SRC)/*.d
	-rm $(STARTUP_SRC)/*.o
	-rm $(APP_SRC)/*.d
	-rm $(APP_SRC)/*.o
	-rm $(ELF)
	-rm $(HEX)
	-rm $(BIN)
