#STM32 makefile template
# 生成的文件名<项目名>
PROJECT                  = ad5791ref_stm32f030

# 定义文件格式和文件名
TARGET                  := $(PROJECT)
TARGET_ELF              := $(TARGET).elf
TARGET_BIN              := $(TARGET).bin
TARGET_HEX              := $(TARGET).hex
OBJCPFLAGS_ELF_TO_BIN    = -Obinary
OBJCPFLAGS_ELF_TO_HEX    = -Oihex
OBJCPFLAGS_BIN_TO_HEX    = -Ibinary -Oihex
VPATH = ..
# 定义路径
TOP_DIR      = .
INC_DIR     := -I./
INC_DIR     += -I../src
INC_DIR 	+= -I../stm32f0xxlib/CMSIS/Include
INC_DIR 	+= -I../stm32f0xxlib/CMSIS/Device/ST/STM32F0xx/Include
INC_DIR 	+= -I../stm32f0xxlib/STM32F0xx_StdPeriph_Driver/inc
INC_DIR 	+= -I./app
INC_DIR 	+= -I./bsp
INC_DIR 	+= --include ./stm32f0xx_conf.h

# 设置ld链接脚本文件
LDSCRIPT    := ./STM32F030F4.ld

# 定义编译工具
CC          = arm-none-eabi-gcc
AS          = arm-none-eabi-as
LD          = arm-none-eabi-ld
AR          = arm-none-eabi-ar
OBJCP       = arm-none-eabi-objcopy

# 定义编译标志
CCFLAGS     += -Wall -mcpu=cortex-m0 -mthumb -g -mfloat-abi=soft '-std=gnu99'
CCFLAGS		+= -Xlinker -Map=output.map 
# -march=armv7-m
ASFLAGS     += -Wall -mcpu=cortex-m0 -mthumb
LDFLAGS     += -T $(LDSCRIPT) -L'C:\Program Files (x86)\GNU Tools Arm Embedded\7 2018-q2-update\lib\gcc\arm-none-eabi\7.3.1' -L'C:\Program Files (x86)\GNU Tools Arm Embedded\7 2018-q2-update\arm-none-eabi\lib' -lgcc -lc -lnosys
LDFLAGS 	+= -Map=output.map 
#-A armv7-m
#LDFLAGS     += -L /Users/ch-yanghl/gcc-arm-none-eabi/lib/gcc/arm-none-eabi/5.4.1
#LDFLAGS     += -L /Users/ch-yanghl/gcc-arm-none-eabi/arm-none-eabi/lib/thumb

# .c文件中的头文件引用查找路径
CCFLAGS     += $(INC_DIR)

# .s文件的flags
#ASFLAGS        +=

# .c文件编译时定义宏
CCFLAGS     += -D STM32F030 -D USE_STDPERIPH_DRIVER
VPATH = ..
SOURCE := $(wildcard ./*.c)
SOURCE += $(wildcard ./bsp/*.c)
SOURCE += $(wildcard ./app/*.c)
SOURCE += $(wildcard ../stm32f0xxlib/STM32F0xx_StdPeriph_Driver/src/*.c)

# 添加启动文件
#SOURCE         += $(SCRIPT_DIR)/startup_stm32f10x_md.c
SOURCE_ASM      := ../src/startup_stm32f030.s

# 展开工作 子目录中的inc文件（inc文件中添加需要编译链接的.c，.s等文件）
#-include $(TOP_DIR)/src/make.inc
#-include $(TOP_DIR)/third_party/make.inc

# 替换文件后缀
C_OBJS      := $(SOURCE:%.c=%.o)
ASM_OBJS    := $(SOURCE_ASM:%.s=%.o)

# 编译命令的定义
COMPILE     = $(CC) $(CCFLAGS) -c $< -o $@ 
ASSEMBLE    = $(AS) $(ASFLAGS) -c $< -o $@ 
LINK        = $(LD) $+ $(LDFLAGS) $(LDLIBS) -o $@ 
ELF_TO_BIN  = $(OBJCP) $(OBJCPFLAGS_ELF_TO_BIN) $< $@
BIN_TO_HEX  = $(OBJCP) $(OBJCPFLAGS_BIN_TO_HEX) $< $@ 

# 定义伪目标
.PHONY: all clean printf

# 定义规则
all: $(TARGET_HEX)
	@echo "build done"

$(TARGET_HEX): $(TARGET_BIN)
	$(BIN_TO_HEX)

$(TARGET_BIN): $(TARGET_ELF)
	$(ELF_TO_BIN)

$(TARGET_ELF): $(C_OBJS) $(ASM_OBJS)
	$(LINK)

$(C_OBJS):%.o:%.c
	$(COMPILE)

$(ASM_OBJS):%.o:%.s
	$(ASSEMBLE) 

printf:
	@echo $(ASM_OBJS)
	@echo $(ASSEMBLE)

# 清理项
clean:
	rm -f $(TARGET_HEX)
	rm -f $(TARGET_BIN)
	rm -f $(TARGET_ELF)
	rm -f $(C_OBJS) $(ASM_OBJS)
	@echo "clean done"