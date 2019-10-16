#STM32 makefile template
PROJECT			= ad5791ref

CPU_FLAG    = -mthumb -mcpu=cortex-m0 -march=armv6-m -mfloat-abi=soft
LDSCRIPT    := ./STM32F030F4.ld

SOURCE      := $(wildcard ./*.c)
SOURCE      += $(wildcard ./bsp/*.c)
SOURCE      += $(wildcard ./app/*.c)
SOURCE      += $(wildcard ./printf/*.c)
SOURCE      += $(wildcard ../stm32f0xxlib/STM32F0xx_StdPeriph_Driver/src/*.c)

SOURCE_ASM  := ../src/startup_stm32f030.s

INC_DIR     := -I./
INC_DIR     += -I../src
INC_DIR 		+= -I../stm32f0xxlib/CMSIS/Include
INC_DIR 		+= -I../stm32f0xxlib/CMSIS/Device/ST/STM32F0xx/Include
INC_DIR 		+= -I../stm32f0xxlib/STM32F0xx_StdPeriph_Driver/inc
INC_DIR 		+= -I./app
INC_DIR 		+= -I./bsp
INC_DIR 		+= -I./printf
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
ASFLAGS     += $(CPU_FLAG) #-Wall

LDFLAGS     := -T $(LDSCRIPT) -nostartfiles -Wl,--gc-sections
LDFLAGS 		+= $(CPU_FLAG) -Xlinker -Map=$(PROJECT).map

C_OBJS      := $(SOURCE:%.c=%.o)
ASM_OBJS    := $(SOURCE_ASM:%.s=%.o)

COMPILE     = $(CC) $(CCFLAGS) -c $< -o $@ 
ASSEMBLE    = $(AS) $(ASFLAGS) -c $< -o $@ 
LINK        = $(LD) $+ $(LDFLAGS) $(LDLIBS) -o $@ 

.PHONY: all clean printf

all: $(PROJECT).bin
	@echo "build done"

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJDUMP) -S $(PROJECT).elf > $(PROJECT).list
	$(OBJCP) -Oihex $(PROJECT).elf $(PROJECT).hex
	$(OBJCP) -Obinary $(PROJECT).elf $(PROJECT).bin

$(PROJECT).elf: $(C_OBJS) $(ASM_OBJS)
	@$(LINK)

$(C_OBJS):%.o:%.c
	@$(COMPILE)

$(ASM_OBJS):%.o:%.s
	$(ASSEMBLE) 

clean:
	rm -f $(C_OBJS) $(ASM_OBJS)
	rm -f $(PROJECT).list  $(PROJECT).map $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin
	@echo "clean done"

flash: $(TARGET_BIN)
# st-flash write $(PROJECT).bin 0x8000000
	@JLink.exe -device stm32f030c6 -if SWD -speed 4000 -CommanderScript .\jlink-script.jlink 
	@echo "bin file downloaded"
erase:
#st-flash erase
	@JLink.exe -device stm32f030c6 -if SWD -speed 4000 -CommanderScript .\jlink-script-erase.jlink
