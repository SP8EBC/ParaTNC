################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/drivers/l4/flash_stm32l4x.c \
../system/src/drivers/l4/i2c_stm32l4x.c \
../system/src/drivers/l4/pwm_input_stm32l4x.c \
../system/src/drivers/l4/serial_stm32l4x.c \
../system/src/drivers/l4/spi_stm32l4xx.c 

OBJS += \
./system/src/drivers/l4/flash_stm32l4x.o \
./system/src/drivers/l4/i2c_stm32l4x.o \
./system/src/drivers/l4/pwm_input_stm32l4x.o \
./system/src/drivers/l4/serial_stm32l4x.o \
./system/src/drivers/l4/spi_stm32l4xx.o 

C_DEPS += \
./system/src/drivers/l4/flash_stm32l4x.d \
./system/src/drivers/l4/i2c_stm32l4x.d \
./system/src/drivers/l4/pwm_input_stm32l4x.d \
./system/src/drivers/l4/serial_stm32l4x.d \
./system/src/drivers/l4/spi_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/drivers/l4/%.o: ../system/src/drivers/l4/%.c system/src/drivers/l4/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wuninitialized -Wall -Wextra -Wlogical-op -Waggregate-return -g3 -DDEBUG -Dno_EVENTLOG_FROM_ISR -Dno_SX1262_SHMIDT_NOT_GATE -Dno_SX1262_IMPLEMENTATION -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../system/freertos/include" -I"../system/freertos" -I"../system/freertos/portable" -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/tm" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


