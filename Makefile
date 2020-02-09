# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = FM_SYNTH

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
App/Src/main.c \
App/Src/ui_task.c \
App/Src/synth_task.c \
App/Src/cli_task.c \
App/Src/cli_cmd.c \
App/Src/stm32g0xx_it.c \
App/Src/stm32g0xx_hal_msp.c \
App/Src/system_stm32g0xx.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_gpio.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_tim.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_tim_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_spi.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_i2c.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_i2c_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_uart.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_uart_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_rcc.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_rcc_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_ll_rcc.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_dma.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_dma_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_pwr.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_pwr_ex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_cortex.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c \
Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_exti.c \
Drivers/User_Drivers/Src/serial_driver.c \
Drivers/User_Drivers/Src/spi_driver.c \
Drivers/User_Drivers/Src/i2c_driver.c \
Drivers/User_Drivers/Src/YM2612_driver.c \
Drivers/User_Drivers/Src/encoder_driver.c \
Drivers/User_Drivers/Src/display_driver.c \
Lib/Src/printf.c \
Lib/Src/circular_buffer.c \
Lib/u8g2/u8g2_bitmap.c \
Lib/u8g2/u8g2_box.c \
Lib/u8g2/u8g2_buffer.c \
Lib/u8g2/u8g2_circle.c \
Lib/u8g2/u8g2_cleardisplay.c \
Lib/u8g2/u8g2_d_memory.c \
Lib/u8g2/u8g2_d_setup.c \
Lib/u8g2/u8g2_font.c \
Lib/u8g2/u8g2_fonts.c \
Lib/u8g2/u8g2_hvline.c \
Lib/u8g2/u8g2_input_value.c \
Lib/u8g2/u8g2_intersection.c \
Lib/u8g2/u8g2_kerning.c \
Lib/u8g2/u8g2_line.c \
Lib/u8g2/u8g2_ll_hvline.c \
Lib/u8g2/u8g2_message.c \
Lib/u8g2/u8g2_polygon.c \
Lib/u8g2/u8g2_selection_list.c \
Lib/u8g2/u8g2_setup.c \
Lib/u8g2/u8log.c \
Lib/u8g2/u8log_u8g2.c \
Lib/u8g2/u8log_u8x8.c \
Lib/u8g2/u8x8_8x8.c \
Lib/u8g2/u8x8_byte.c \
Lib/u8g2/u8x8_cad.c \
Lib/u8g2/u8x8_capture.c \
Lib/u8g2/u8x8_d_ssd1306_128x64_noname.c \
Lib/u8g2/u8x8_debounce.c \
Lib/u8g2/u8x8_display.c \
Lib/u8g2/u8x8_fonts.c \
Lib/u8g2/u8x8_gpio.c \
Lib/u8g2/u8x8_input_value.c \
Lib/u8g2/u8x8_message.c \
Lib/u8g2/u8x8_selection_list.c \
Lib/u8g2/u8x8_setup.c \
Lib/u8g2/u8x8_string.c \
Lib/u8g2/u8x8_u8toa.c \
Lib/u8g2/u8x8_u16toa.c \
RTOS/FreeRTOS/Source/croutine.c \
RTOS/FreeRTOS/Source/event_groups.c \
RTOS/FreeRTOS/Source/list.c \
RTOS/FreeRTOS/Source/queue.c \
RTOS/FreeRTOS/Source/stream_buffer.c \
RTOS/FreeRTOS/Source/tasks.c \
RTOS/FreeRTOS/Source/timers.c \
RTOS/FreeRTOS/Source/portable/GCC/ARM_CM0/port.c \
RTOS/FreeRTOS/Source/portable/MemMang/heap_4.c \
RTOS/FreeRTOS-Plus-CLI/FreeRTOS_CLI.c \

# ASM sources
ASM_SOURCES =  \
startup_stm32g070xx.s

#######################################
# binaries
#######################################
GCC_PATH = C:\Program Files (x86)\GNU Tools Arm Embedded\9 2019-q4-major\bin
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

# Clean tool
RM = del /q

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0plus

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32G070xx

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-IApp/Inc \
-ILib/Inc \
-ILib/u8g2 \
-IDrivers/User_Drivers/Inc \
-IDrivers/STM32G0xx_HAL_Driver/Inc \
-IDrivers/STM32G0xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32G0xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/CMSIS/Include \
-IRTOS/FreeRTOS/Source/include \
-IRTOS/FreeRTOS/Source/portable/GCC/ARM_CM0 \
-IRTOS/FreeRTOS-Plus-CLI \

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32G070CBTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
#	-rm -fR $(BUILD_DIR)
	$(RM) $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***