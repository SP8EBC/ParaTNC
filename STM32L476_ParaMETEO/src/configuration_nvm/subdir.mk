################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/configuration_nvm/config_data_default.c \
../src/configuration_nvm/config_data_first.c \
../src/configuration_nvm/config_data_second.c \
../src/configuration_nvm/configuration_handler.c 

OBJS += \
./src/configuration_nvm/config_data_default.o \
./src/configuration_nvm/config_data_first.o \
./src/configuration_nvm/config_data_second.o \
./src/configuration_nvm/configuration_handler.o 

C_DEPS += \
./src/configuration_nvm/config_data_default.d \
./src/configuration_nvm/config_data_first.d \
./src/configuration_nvm/config_data_second.d \
./src/configuration_nvm/configuration_handler.d 


# Each subdirectory must supply rules for building sources it contributes
src/configuration_nvm/%.o: ../src/configuration_nvm/%.c src/configuration_nvm/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


