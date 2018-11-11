#STM32 makefile template
PROJECT			= ad5791ref_stm32f030

CPU_FLAG    = -mthumb -mcpu=cortex-m0 -march=armv6-m -mfloat-abi=soft
LDSCRIPT    := ./STM32F030F4.ld

SOURCE      := $(wildcard ./*.c)
SOURCE      += $(wildcard ./bsp/*.c)
SOURCE      += $(wildcard ./app/*.c)
SOURCE      += $(wildcard ../stm32f0xxlib/STM32F0xx_StdPeriph_Driver/src/*.c)

SOURCE_ASM  := ../src/startup_stm32f030.s

TARGET                  := $(PROJECT)
TARGET_ELF              := $(TARGET).elf
TARGET_BIN              := $(TARGET).bin
TARGET_HEX              := $(TARGET).hex
OBJCPFLAGS_ELF_TO_BIN    = -Obinary
OBJCPFLAGS_ELF_TO_HEX    = -O ihex
OBJCPFLAGS_BIN_TO_HEX    = -Ibinary -Oihex
OBJCPFLAGS_HEX_TO_BIN    = -Iihex -Obinary

INC_DIR     := -I./
INC_DIR     += -I../src
INC_DIR 		+= -I../stm32f0xxlib/CMSIS/Include
INC_DIR 		+= -I../stm32f0xxlib/CMSIS/Device/ST/STM32F0xx/Include
INC_DIR 		+= -I../stm32f0xxlib/STM32F0xx_StdPeriph_Driver/inc
INC_DIR 		+= -I./app
INC_DIR 		+= -I./bsp
INC_DIR 		+= --include ./stm32f0xx_conf.h

CC          = arm-none-eabi-gcc
AS          = arm-none-eabi-as
LD          = arm-none-eabi-gcc
AR          = arm-none-eabi-ar
OBJCP       = arm-none-eabi-objcopy
OBJDUMP			= arm-none-eabi-objdump

CCFLAGS			:= -D STM32F030 -D USE_STDPERIPH_DRIVER
CCFLAGS     += $(CPU_FLAG) -Wall -g '-std=gnu99' -O2 -fno-common 
CCFLAGS     += $(INC_DIR)
ASFLAGS     += $(CPU_FLAG) -Wall

LDFLAGS     := -T $(LDSCRIPT) -nostartfiles -Wl,--gc-sections
LDFLAGS 		+= $(CPU_FLAG) -Xlinker -Map=$(PROJECT).map

C_OBJS      := $(SOURCE:%.c=%.o)
ASM_OBJS    := $(SOURCE_ASM:%.s=%.o)

COMPILE     = $(CC) $(CCFLAGS) -c $< -o $@ 
ASSEMBLE    = $(AS) $(ASFLAGS) -c $< -o $@ 
LINK        = $(LD) $+ $(LDFLAGS) $(LDLIBS) -o $@ 
ELF_TO_BIN  = $(OBJCP) $(OBJCPFLAGS_ELF_TO_BIN) $< $@
ELF_TO_HEX  = $(OBJCP) $(OBJCPFLAGS_ELF_TO_HEX) $< $@

.PHONY: all clean printf

all: $(TARGET_ELF)
	@echo "build done"

$(TARGET_ELF): $(C_OBJS) $(ASM_OBJS)
	$(LINK)
	$(ELF_TO_HEX)
	$(ELF_TO_BIN)
	$(OBJDUMP) -S $(TARGET_ELF) > $(PROJECT).list 

$(C_OBJS):%.o:%.c
	$(COMPILE)

$(ASM_OBJS):%.o:%.s
	$(ASSEMBLE) 

clean:
	rm -f $(TARGET_HEX)
	rm -f $(TARGET_BIN)
	rm -f $(TARGET_ELF)
	rm -f $(C_OBJS) $(ASM_OBJS)
	@echo "clean done"

flash: $(TARGET_BIN)
	st-flash erase
	st-flash write $(PROJECT).bin 0x8000000
erase:
	st-flash erase