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
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants  -g3 -DDEBUG -DTRACE -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


