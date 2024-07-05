################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/stored_configuration_nvm/config_data_default.c \
../src/stored_configuration_nvm/config_data_first.c \
../src/stored_configuration_nvm/config_data_second.c \
../src/stored_configuration_nvm/configuration_handler.c 

OBJS += \
./src/stored_configuration_nvm/config_data_default.o \
./src/stored_configuration_nvm/config_data_first.o \
./src/stored_configuration_nvm/config_data_second.o \
./src/stored_configuration_nvm/configuration_handler.o 

C_DEPS += \
./src/stored_configuration_nvm/config_data_default.d \
./src/stored_configuration_nvm/config_data_first.d \
./src/stored_configuration_nvm/config_data_second.d \
./src/stored_configuration_nvm/configuration_handler.d 


# Each subdirectory must supply rules for building sources it contributes
src/stored_configuration_nvm/%.o: ../src/stored_configuration_nvm/%.c src/stored_configuration_nvm/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -DDEBUG -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


