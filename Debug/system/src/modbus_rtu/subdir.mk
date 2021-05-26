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
system/src/modbus_rtu/%.o: ../system/src/modbus_rtu/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants  -g3 -DDEBUG -DTRACE -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


