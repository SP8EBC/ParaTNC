################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/gsm/sim800c.c \
../system/src/gsm/sim800c_engineering.c \
../system/src/gsm/sim800c_gprs.c \
../system/src/gsm/sim800c_poolers.c \
../system/src/gsm/sim800c_tcpip.c 

OBJS += \
./system/src/gsm/sim800c.o \
./system/src/gsm/sim800c_engineering.o \
./system/src/gsm/sim800c_gprs.o \
./system/src/gsm/sim800c_poolers.o \
./system/src/gsm/sim800c_tcpip.o 

C_DEPS += \
./system/src/gsm/sim800c.d \
./system/src/gsm/sim800c_engineering.d \
./system/src/gsm/sim800c_gprs.d \
./system/src/gsm/sim800c_poolers.d \
./system/src/gsm/sim800c_tcpip.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/gsm/%.o: ../system/src/gsm/%.c system/src/gsm/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


