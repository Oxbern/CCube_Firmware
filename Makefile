# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=TestLTDC
BUILD=build

##########
# DEFINES
##########
#DEFINES = -DJSMN_PARENT_LINKS -DUSE_USB_FS
######## - DEFINES

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

CFLAGS += -lrdimon #--specs=rdimon.specs 
CFLAGS += $(DEFINES)

######## - COMMON CFLAGS

#########
# ARM DEF
#########
CFLAGS += -IDrivers/CMSIS/Include/
######## - ARM DEF

##################
# STM32F4xx Drivers
##################

CFLAGS += -IDrivers/CMSIS/Device/ST/STM32F4xx/Include
SRCS += Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

CFLAGS += -IDrivers/STM32F4xx_HAL_Driver/Inc
SRCS += $(wildcard Drivers/STM32F4xx_HAL_Driver/Src/*.c)

######## - STM32F4xx Drivers

########################
# specific to STM32F429
########################

DEFS = -DSTM32F429xx

SRCS += Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f429xx.s

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
# STemWin
#################
CFLAGS += -Igui/stemwin/user
CFLAGS += -Igui/stemwin/inc
LIBS   += -Lgui/stemwin/lib
LDFLAGS += -lSTemWin522_CM4_GCC
SRCS += $(wildcard gui/stemwin/user/*.c)

#################
# FATFS
#################
CFLAGS += -Ifs/fatfs/inc
SRCS += $(wildcard fs/fatfs/src/*.c)
SRCS += fs/fatfs/src/option/syscall.c
SRCS += fs/fatfs/src/option/unicode.c

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
	@echo "Removing *_template.* files......"
	@rm -f Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_conf_template.h Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_msp_template.c
	@echo "Compiling project....."
	@$(CC) $(CFLAGS) $(DEFS) $(LIBS) $^ -o $(BUILD)/$@ $(LDFLAGS)
	@$(OBJCOPY) -O ihex $(BUILD)/$(PROJ_NAME).elf $(BUILD)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(BUILD)/$(PROJ_NAME).elf $(BUILD)/$(PROJ_NAME).bin
	@echo "DONE"

dep:
	$(CC) $(CFLAGS) $(DEFS) $(LIBS) -M system/main.c $(LDFLAGS)

clean:
	@echo "Cleaning object files and binaries....."
	@rm -f *.o $(BUILD)/*
	@echo "DONE"

# Flash the STM32F4
burn: proj
	$(STLINK)/st-flash write $(BUILD)/$(PROJ_NAME).bin 0x8000000

gdb:
	arm-none-eabi-gdb --eval-command="target extended-remote localhost:4242" $(BUILD)/$(PROJ_NAME).elf

erase:	
	$(STLINK)/st-flash erase
