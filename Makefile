# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=TestLTDC
BUILD=build

##########
# STLINK
##########
STLINK=/home/elkhadiy/stlink
######## - STLINK

#############
# cross tools
#############
CC=arm-none-eabi-gcc
LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
######## - cross tools

################
# COMMON CFLAGS
################

CFLAGS  = -g -Wall -std=gnu99
CFLAGS += -T$(LDSCRIPT)
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -I.
CFLAGS += -I/usr/arm-none-eabi/lib

CFLAGS += -lrdimon
CFLAGS += $(DEFINES)

######## - COMMON CFLAGS

#########
# ARM DEF
#########
CFLAGS += -Icmsis
######## - ARM DEF

##################
# STM32F4xx Drivers
##################

CFLAGS += -Icmsis/stm32f4xx
SRCS += cmsis/stm32f4xx/system_stm32f4xx.c

CFLAGS += -Ihal/inc
SRCS += $(wildcard hal/src/*.c)

######## - STM32F4xx Drivers

########################
# specific to STM32F429
########################

DEFS = -DSTM32F429xx

SRCS += cmsis/stm32f4xx/startup_stm32f429xx.s

LDSCRIPT = flash.ld

######## - specific to STM32F429

#################
# USB
#################
CFLAGS += -Iusb/core
SRCS += $(wildcard usb/core/*.c)
CFLAGS += -Iusb/class
SRCS += $(wildcard usb/class/*.c)
CFLAGS += -Iusb/user
SRCS += $(wildcard usb/user/*.c)

#################
# LTDC
#################
CFLAGS += -Igui
SRCS += $(wildcard gui/*.c)

#################
# STemWin
#################
CFLAGS += -Igui/stemwin/user
CFLAGS += -Igui/stemwin/inc
LIBS   += -Lgui/stemwin/lib
LDFLAGS += -lSTemWin522_CM4_GCC
SRCS += $(wildcard gui/stemwin/user/*.c)

#################
# SDIO
#################
CFLAGS += -Ifs
SRCS += $(wildcard fs/*.c)

#################
# FATFS
#################
CFLAGS += -Ifs/fatfs/inc
SRCS += $(wildcard fs/fatfs/src/*.c)
SRCS += fs/fatfs/src/option/syscall.c
SRCS += fs/fatfs/src/option/unicode.c

#################
# FreeRTOS
#################
CFLAGS += -Ifreertos/inc
SRCS += $(wildcard freertos/src/*.c)

#################
# LED
#################
CFLAGS += -Iled
SRCS += $(wildcard led/*.c)

#################
# System
#################
CFLAGS += -Isystem
SRCS += $(wildcard system/*.c)

##########
# Rules
##########
OBJS = $(SRCS:.c=.o)

.PHONY: proj

all: proj

proj: $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	@echo "Compiling project..."
	@$(CC) $(CFLAGS) $(DEFS) $(LIBS) $^ -o $(BUILD)/$@ $(LDFLAGS)
	@$(OBJCOPY) -O ihex $(BUILD)/$(PROJ_NAME).elf $(BUILD)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(BUILD)/$(PROJ_NAME).elf $(BUILD)/$(PROJ_NAME).bin
	@echo "DONE"

dep:
	$(CC) $(CFLAGS) $(DEFS) $(LIBS) -M system/main.c $(LDFLAGS)

clean:
	@echo "Cleaning object files and binaries..."
	@rm -f *.o $(BUILD)/*
	@echo "DONE"

# Flash the STM32F4
burn: proj
	$(STLINK)/st-flash write $(BUILD)/$(PROJ_NAME).bin 0x8000000

gdb:
	arm-none-eabi-gdb --eval-command="target extended-remote localhost:4242" $(BUILD)/$(PROJ_NAME).elf

erase:	
	$(STLINK)/st-flash erase
