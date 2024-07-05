################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/cmsis/stm32l4xx/stm32l4xx_it.c \
../system/src/cmsis/stm32l4xx/system_stm32l4xx.c 

S_UPPER_SRCS += \
../system/src/cmsis/stm32l4xx/startup_stm32l471xx.S 

OBJS += \
./system/src/cmsis/stm32l4xx/startup_stm32l471xx.o \
./system/src/cmsis/stm32l4xx/stm32l4xx_it.o \
./system/src/cmsis/stm32l4xx/system_stm32l4xx.o 

S_UPPER_DEPS += \
./system/src/cmsis/stm32l4xx/startup_stm32l471xx.d 

C_DEPS += \
./system/src/cmsis/stm32l4xx/stm32l4xx_it.d \
./system/src/cmsis/stm32l4xx/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/cmsis/stm32l4xx/%.o: ../system/src/cmsis/stm32l4xx/%.S system/src/cmsis/stm32l4xx/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -x assembler-with-cpp -DDEBUG -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -USTM32F10X_MD_VL -I"../include" -I"../include/etc" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

system/src/cmsis/stm32l4xx/%.o: ../system/src/cmsis/stm32l4xx/%.c system/src/cmsis/stm32l4xx/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -DDEBUG -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


