################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/aprs/adc.c \
../system/src/aprs/afsk.c \
../system/src/aprs/afsk_pr.c \
../system/src/aprs/ax25.c \
../system/src/aprs/beacon.c \
../system/src/aprs/crc.c \
../system/src/aprs/dac.c \
../system/src/aprs/digi.c \
../system/src/aprs/message.c \
../system/src/aprs/status.c \
../system/src/aprs/telemetry.c \
../system/src/aprs/wx.c 

OBJS += \
./system/src/aprs/adc.o \
./system/src/aprs/afsk.o \
./system/src/aprs/afsk_pr.o \
./system/src/aprs/ax25.o \
./system/src/aprs/beacon.o \
./system/src/aprs/crc.o \
./system/src/aprs/dac.o \
./system/src/aprs/digi.o \
./system/src/aprs/message.o \
./system/src/aprs/status.o \
./system/src/aprs/telemetry.o \
./system/src/aprs/wx.o 

C_DEPS += \
./system/src/aprs/adc.d \
./system/src/aprs/afsk.d \
./system/src/aprs/afsk_pr.d \
./system/src/aprs/ax25.d \
./system/src/aprs/beacon.d \
./system/src/aprs/crc.d \
./system/src/aprs/dac.d \
./system/src/aprs/digi.d \
./system/src/aprs/message.d \
./system/src/aprs/status.d \
./system/src/aprs/telemetry.d \
./system/src/aprs/wx.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/aprs/%.o: ../system/src/aprs/%.c system/src/aprs/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wuninitialized -Wall -Wextra -Wlogical-op -Waggregate-return -g3 -DDEBUG -Dno_SX1262_SHMIDT_NOT_GATE -Dno_SX1262_IMPLEMENTATION -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../system/freertos/include" -I"../system/freertos" -I"../system/freertos/portable" -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/tm" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


