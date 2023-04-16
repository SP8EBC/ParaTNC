################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/LedConfig.c \
../src/PathConfig.c \
../src/TimerConfig.c \
../src/_write.c \
../src/api.c \
../src/aprsis.c \
../src/config_data_default.c \
../src/config_data_first.c \
../src/config_data_second.c \
../src/configuration_handler.c \
../src/delay.c \
../src/dummy.c \
../src/float_to_string.c \
../src/io.c \
../src/it_handlers.c \
../src/kiss_callback.c \
../src/kiss_communication.c \
../src/main.c \
../src/nvm.c \
../src/packet_tx_handler.c \
../src/pwr_save.c \
../src/rte_main.c \
../src/rte_pv.c \
../src/rte_pwr.c \
../src/rte_rtu.c \
../src/rte_wx.c \
../src/wx_handler.c \
../src/wx_handler_humidity.c \
../src/wx_handler_pressure.c \
../src/wx_handler_temperature.c \
../src/wx_pwr_switch.c 

OBJS += \
./src/LedConfig.o \
./src/PathConfig.o \
./src/TimerConfig.o \
./src/_write.o \
./src/api.o \
./src/aprsis.o \
./src/config_data_default.o \
./src/config_data_first.o \
./src/config_data_second.o \
./src/configuration_handler.o \
./src/delay.o \
./src/dummy.o \
./src/float_to_string.o \
./src/io.o \
./src/it_handlers.o \
./src/kiss_callback.o \
./src/kiss_communication.o \
./src/main.o \
./src/nvm.o \
./src/packet_tx_handler.o \
./src/pwr_save.o \
./src/rte_main.o \
./src/rte_pv.o \
./src/rte_pwr.o \
./src/rte_rtu.o \
./src/rte_wx.o \
./src/wx_handler.o \
./src/wx_handler_humidity.o \
./src/wx_handler_pressure.o \
./src/wx_handler_temperature.o \
./src/wx_pwr_switch.o 

C_DEPS += \
./src/LedConfig.d \
./src/PathConfig.d \
./src/TimerConfig.d \
./src/_write.d \
./src/api.d \
./src/aprsis.d \
./src/config_data_default.d \
./src/config_data_first.d \
./src/config_data_second.d \
./src/configuration_handler.d \
./src/delay.d \
./src/dummy.d \
./src/float_to_string.d \
./src/io.d \
./src/it_handlers.d \
./src/kiss_callback.d \
./src/kiss_communication.d \
./src/main.d \
./src/nvm.d \
./src/packet_tx_handler.d \
./src/pwr_save.d \
./src/rte_main.d \
./src/rte_pv.d \
./src/rte_pwr.d \
./src/rte_rtu.d \
./src/rte_wx.d \
./src/wx_handler.d \
./src/wx_handler_humidity.d \
./src/wx_handler_pressure.d \
./src/wx_handler_temperature.d \
./src/wx_pwr_switch.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


