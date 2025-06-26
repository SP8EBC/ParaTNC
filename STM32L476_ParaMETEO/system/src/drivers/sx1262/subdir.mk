################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/drivers/sx1262/sx1262.c \
../system/src/drivers/sx1262/sx1262_data_io.c \
../system/src/drivers/sx1262/sx1262_internals.c \
../system/src/drivers/sx1262/sx1262_irq_dio.c \
../system/src/drivers/sx1262/sx1262_modes.c \
../system/src/drivers/sx1262/sx1262_rf.c \
../system/src/drivers/sx1262/sx1262_status.c 

OBJS += \
./system/src/drivers/sx1262/sx1262.o \
./system/src/drivers/sx1262/sx1262_data_io.o \
./system/src/drivers/sx1262/sx1262_internals.o \
./system/src/drivers/sx1262/sx1262_irq_dio.o \
./system/src/drivers/sx1262/sx1262_modes.o \
./system/src/drivers/sx1262/sx1262_rf.o \
./system/src/drivers/sx1262/sx1262_status.o 

C_DEPS += \
./system/src/drivers/sx1262/sx1262.d \
./system/src/drivers/sx1262/sx1262_data_io.d \
./system/src/drivers/sx1262/sx1262_internals.d \
./system/src/drivers/sx1262/sx1262_irq_dio.d \
./system/src/drivers/sx1262/sx1262_modes.d \
./system/src/drivers/sx1262/sx1262_rf.d \
./system/src/drivers/sx1262/sx1262_status.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/drivers/sx1262/%.o: ../system/src/drivers/sx1262/%.c system/src/drivers/sx1262/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wuninitialized -Wall -Wextra -Wlogical-op -Waggregate-return -g3 -DDEBUG -DSX1262_IMPLEMENTATION -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/tm" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


