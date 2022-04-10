################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/umb_master/umb_0x23_offline_data.c \
../system/src/umb_master/umb_0x26_status.c \
../system/src/umb_master/umb_channel_pool.c \
../system/src/umb_master/umb_master.c 

OBJS += \
./system/src/umb_master/umb_0x23_offline_data.o \
./system/src/umb_master/umb_0x26_status.o \
./system/src/umb_master/umb_channel_pool.o \
./system/src/umb_master/umb_master.o 

C_DEPS += \
./system/src/umb_master/umb_0x23_offline_data.d \
./system/src/umb_master/umb_0x26_status.d \
./system/src/umb_master/umb_channel_pool.d \
./system/src/umb_master/umb_master.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/umb_master/%.o: ../system/src/umb_master/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants  -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


