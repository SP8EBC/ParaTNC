################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/drivers/f1/gpio_conf_stm32f1x.c \
../system/src/drivers/f1/i2c_stm32f1x.c \
../system/src/drivers/f1/serial_stm32f1x.c 

OBJS += \
./system/src/drivers/f1/gpio_conf_stm32f1x.o \
./system/src/drivers/f1/i2c_stm32f1x.o \
./system/src/drivers/f1/serial_stm32f1x.o 

C_DEPS += \
./system/src/drivers/f1/gpio_conf_stm32f1x.d \
./system/src/drivers/f1/i2c_stm32f1x.d \
./system/src/drivers/f1/serial_stm32f1x.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/drivers/f1/%.o: ../system/src/drivers/f1/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants  -g3 -DDEBUG -DTRACE -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


