################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/davis_vantage/davis.c \
../system/src/davis_vantage/davis_parsers.c 

OBJS += \
./system/src/davis_vantage/davis.o \
./system/src/davis_vantage/davis_parsers.o 

C_DEPS += \
./system/src/davis_vantage/davis.d \
./system/src/davis_vantage/davis_parsers.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/davis_vantage/%.o: ../system/src/davis_vantage/%.c system/src/davis_vantage/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -DDEBUG -DPARATNC -DTRACE -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -UPARAMETEO -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


