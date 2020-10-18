################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/KissCommunication.c \
../src/LedConfig.c \
../src/PathConfig.c \
../src/TimerConfig.c \
../src/_write.c \
../src/delay.c \
../src/io.c \
../src/it_handlers.c \
../src/main.c \
../src/packet_tx_handler.c \
../src/rte_main.c \
../src/rte_pv.c \
../src/rte_wx.c \
../src/wx_handler.c 

OBJS += \
./src/KissCommunication.o \
./src/LedConfig.o \
./src/PathConfig.o \
./src/TimerConfig.o \
./src/_write.o \
./src/delay.o \
./src/io.o \
./src/it_handlers.o \
./src/main.o \
./src/packet_tx_handler.o \
./src/rte_main.o \
./src/rte_pv.o \
./src/rte_wx.o \
./src/wx_handler.o 

C_DEPS += \
./src/KissCommunication.d \
./src/LedConfig.d \
./src/PathConfig.d \
./src/TimerConfig.d \
./src/_write.d \
./src/delay.d \
./src/io.d \
./src/it_handlers.d \
./src/main.d \
./src/packet_tx_handler.d \
./src/rte_main.d \
./src/rte_pv.d \
./src/rte_wx.d \
./src/wx_handler.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants  -g3 -DDEBUG -DTRACE -DSTM32F10X_MD_VL -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wunused-function -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


