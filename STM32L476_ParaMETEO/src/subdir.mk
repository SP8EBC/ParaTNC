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
../src/backup_registers.c \
../src/button_parameteo.c \
../src/debug_hardfault.c \
../src/delay.c \
../src/dummy.c \
../src/event_log.c \
../src/event_log_postmortem.c \
../src/fanet_app.c \
../src/gsm_comm_state_handler.c \
../src/io.c \
../src/it_handlers.c \
../src/main.c \
../src/ntp.c \
../src/packet_tx_handler.c \
../src/pwr_save.c \
../src/rte_main.c \
../src/rte_pv.c \
../src/rte_pwr.c \
../src/rte_rtu.c \
../src/rte_wx.c \
../src/software_version.c \
../src/supervisor.c \
../src/variant_parameteo.c \
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
./src/backup_registers.o \
./src/button_parameteo.o \
./src/debug_hardfault.o \
./src/delay.o \
./src/dummy.o \
./src/event_log.o \
./src/event_log_postmortem.o \
./src/fanet_app.o \
./src/gsm_comm_state_handler.o \
./src/io.o \
./src/it_handlers.o \
./src/main.o \
./src/ntp.o \
./src/packet_tx_handler.o \
./src/pwr_save.o \
./src/rte_main.o \
./src/rte_pv.o \
./src/rte_pwr.o \
./src/rte_rtu.o \
./src/rte_wx.o \
./src/software_version.o \
./src/supervisor.o \
./src/variant_parameteo.o \
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
./src/backup_registers.d \
./src/button_parameteo.d \
./src/debug_hardfault.d \
./src/delay.d \
./src/dummy.d \
./src/event_log.d \
./src/event_log_postmortem.d \
./src/fanet_app.d \
./src/gsm_comm_state_handler.d \
./src/io.d \
./src/it_handlers.d \
./src/main.d \
./src/ntp.d \
./src/packet_tx_handler.d \
./src/pwr_save.d \
./src/rte_main.d \
./src/rte_pv.d \
./src/rte_pwr.d \
./src/rte_rtu.d \
./src/rte_wx.d \
./src/software_version.d \
./src/supervisor.d \
./src/variant_parameteo.d \
./src/wx_handler.d \
./src/wx_handler_humidity.d \
./src/wx_handler_pressure.d \
./src/wx_handler_temperature.d \
./src/wx_pwr_switch.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wuninitialized -Wall -Wextra -Wlogical-op -Waggregate-return -g3 -DDEBUG -DSX1262_D_STORE_LAST_RX_BUFF -DSX1262_IMPLEMENTATION -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/tm" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


