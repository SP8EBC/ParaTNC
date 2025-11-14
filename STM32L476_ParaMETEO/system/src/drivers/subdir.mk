################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/drivers/analog_anemometer.c \
../system/src/drivers/bme280.c \
../system/src/drivers/dallas.c \
../system/src/drivers/dma_helper_functions.c \
../system/src/drivers/max31865.c \
../system/src/drivers/ms5611.c 

OBJS += \
./system/src/drivers/analog_anemometer.o \
./system/src/drivers/bme280.o \
./system/src/drivers/dallas.o \
./system/src/drivers/dma_helper_functions.o \
./system/src/drivers/max31865.o \
./system/src/drivers/ms5611.o 

C_DEPS += \
./system/src/drivers/analog_anemometer.d \
./system/src/drivers/bme280.d \
./system/src/drivers/dallas.d \
./system/src/drivers/dma_helper_functions.d \
./system/src/drivers/max31865.d \
./system/src/drivers/ms5611.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/drivers/%.o: ../system/src/drivers/%.c system/src/drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wuninitialized -Wall -Wextra -Wlogical-op -Waggregate-return -g3 -DDEBUG -DSPRINTF_LONG_LONG -Dno_EVENTLOG_FROM_ISR -Dno_SX1262_SHMIDT_NOT_GATE -Dno_SX1262_IMPLEMENTATION -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../system/freertos/include" -I"../system/freertos" -I"../system/freertos/portable" -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/tm" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


