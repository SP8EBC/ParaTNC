################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/drivers/_dht22.c \
../system/src/drivers/analog_anemometer.c \
../system/src/drivers/dallas.c \
../system/src/drivers/dma_helper_functions.c \
../system/src/drivers/gpio_conf.c \
../system/src/drivers/i2c.c \
../system/src/drivers/ms5611.c \
../system/src/drivers/serial.c \
../system/src/drivers/tx20.c \
../system/src/drivers/user_interf.c 

OBJS += \
./system/src/drivers/_dht22.o \
./system/src/drivers/analog_anemometer.o \
./system/src/drivers/dallas.o \
./system/src/drivers/dma_helper_functions.o \
./system/src/drivers/gpio_conf.o \
./system/src/drivers/i2c.o \
./system/src/drivers/ms5611.o \
./system/src/drivers/serial.o \
./system/src/drivers/tx20.o \
./system/src/drivers/user_interf.o 

C_DEPS += \
./system/src/drivers/_dht22.d \
./system/src/drivers/analog_anemometer.d \
./system/src/drivers/dallas.d \
./system/src/drivers/dma_helper_functions.d \
./system/src/drivers/gpio_conf.d \
./system/src/drivers/i2c.d \
./system/src/drivers/ms5611.d \
./system/src/drivers/serial.d \
./system/src/drivers/tx20.d \
./system/src/drivers/user_interf.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/drivers/%.o: ../system/src/drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


