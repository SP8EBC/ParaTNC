################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/modbus_rtu/rtu_getters.c \
../system/src/modbus_rtu/rtu_parser.c \
../system/src/modbus_rtu/rtu_request.c \
../system/src/modbus_rtu/rtu_serial_io.c 

OBJS += \
./system/src/modbus_rtu/rtu_getters.o \
./system/src/modbus_rtu/rtu_parser.o \
./system/src/modbus_rtu/rtu_request.o \
./system/src/modbus_rtu/rtu_serial_io.o 

C_DEPS += \
./system/src/modbus_rtu/rtu_getters.d \
./system/src/modbus_rtu/rtu_parser.d \
./system/src/modbus_rtu/rtu_request.d \
./system/src/modbus_rtu/rtu_serial_io.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/modbus_rtu/%.o: ../system/src/modbus_rtu/%.c system/src/modbus_rtu/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wuninitialized -Wall -Wextra -Wlogical-op -Waggregate-return -g3 -DDEBUG -DSX1262_IMPLEMENTATION -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/tm" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


