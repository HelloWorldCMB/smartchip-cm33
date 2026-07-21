########################################
# Cortex-M33 yaoxin firmware - GCC build
#
# Usage:
#   make
#   make clean
#
# Output under build/:
#   cm33-freertos.elf / .bin / .hex / .map
########################################

ROOT       := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR  := $(ROOT)/build
GCC_DIR    := $(ROOT)/gcc

CROSS      ?= arm-none-eabi-
CC         := $(CROSS)gcc
AS         := $(CROSS)gcc
OBJCOPY    := $(CROSS)objcopy
SIZE       := $(CROSS)size

TARGET     := cm33-freertos

ifneq ($(wildcard /opt/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux/bin/arm-none-eabi-gcc),)
  export PATH := /opt/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux/bin:$(PATH)
endif

CPU_FLAGS  := -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard
DEFS       := -DARMCM33_DSP_FP -DMSC_CLOCK -D__DBG_ITM

INCLUDES   := \
	-I$(ROOT)/Drivers/include \
	-I$(ROOT)/Drivers/include/leo \
	-I$(ROOT)/Drivers/include/CMSIS \
	-I$(ROOT)/RTE/Include \
	-I$(ROOT)/RTE/_Target_1 \
	-I$(ROOT)/CMSIS/Core/Include \
	-I$(ROOT)/CMSIS/Device/Include \
	-I$(GCC_DIR) \
	-include $(GCC_DIR)/gcc_compat.h

CFLAGS     := $(CPU_FLAGS) $(DEFS) $(INCLUDES) \
	-O2 -g3 -ffunction-sections -fdata-sections \
	-Wall -Wno-unused-parameter -Wno-unused-variable \
	-std=gnu99

ASFLAGS    := $(CPU_FLAGS) $(DEFS) -g3

LDSCRIPT   := $(GCC_DIR)/cm33.ld
LDFLAGS    := $(CPU_FLAGS) -T$(LDSCRIPT) \
	-Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
	--specs=nano.specs --specs=nosys.specs \
	-Wl,--print-memory-usage

C_SRCS := \
	src/main.c \
	src/yaoxin.c \
	Drivers/src/DrvUART010.c \
	Drivers/src/DrvGPIO010.c \
	Drivers/src/DrvSemaphore.c \
	Drivers/src/DrvPWMTMR010.c \
	RTE/Device/ARMCM33_DSP_FP/system_ARMCM33.c \
	gcc/yaoxin_timer.c

ASM_SRCS := gcc/startup_ARMCM33.S

OBJS := $(addprefix $(BUILD_DIR)/,$(C_SRCS:.c=.o) $(ASM_SRCS:.S=.o))

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex
	@$(SIZE) $(BUILD_DIR)/$(TARGET).elf
	@echo "Built: $(BUILD_DIR)/$(TARGET).bin"

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.o: $(ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ROOT)/%.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
