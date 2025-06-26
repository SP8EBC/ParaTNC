################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/crc_.c \
../system/src/float_average.c \
../system/src/float_to_string.c \
../system/src/int_average.c 

OBJS += \
./system/src/crc_.o \
./system/src/float_average.o \
./system/src/float_to_string.o \
./system/src/int_average.o 

C_DEPS += \
./system/src/crc_.d \
./system/src/float_average.d \
./system/src/float_to_string.d \
./system/src/int_average.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/%.o: ../system/src/%.c system/src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wuninitialized -Wall -Wextra -Wlogical-op -Waggregate-return -g3 -DDEBUG -DSX1262_IMPLEMENTATION -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/tm" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


