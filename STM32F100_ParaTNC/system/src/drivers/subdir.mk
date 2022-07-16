################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/drivers/analog_anemometer.c \
../system/src/drivers/bme280.c \
../system/src/drivers/dallas.c \
../system/src/drivers/dma_helper_functions.c \
../system/src/drivers/ms5611.c 

OBJS += \
./system/src/drivers/analog_anemometer.o \
./system/src/drivers/bme280.o \
./system/src/drivers/dallas.o \
./system/src/drivers/dma_helper_functions.o \
./system/src/drivers/ms5611.o 

C_DEPS += \
./system/src/drivers/analog_anemometer.d \
./system/src/drivers/bme280.d \
./system/src/drivers/dallas.d \
./system/src/drivers/dma_helper_functions.d \
./system/src/drivers/ms5611.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/drivers/%.o: ../system/src/drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants  -g3 -DDEBUG -DTRACE -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


