################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/kiss_protocol/kiss_callback.c \
../src/kiss_protocol/kiss_communication.c \
../src/kiss_protocol/kiss_communication_aprsmsg.c \
../src/kiss_protocol/kiss_did.c \
../src/kiss_protocol/kiss_nrc_response.c \
../src/kiss_protocol/kiss_security_access.c 

OBJS += \
./src/kiss_protocol/kiss_callback.o \
./src/kiss_protocol/kiss_communication.o \
./src/kiss_protocol/kiss_communication_aprsmsg.o \
./src/kiss_protocol/kiss_did.o \
./src/kiss_protocol/kiss_nrc_response.o \
./src/kiss_protocol/kiss_security_access.o 

C_DEPS += \
./src/kiss_protocol/kiss_callback.d \
./src/kiss_protocol/kiss_communication.d \
./src/kiss_protocol/kiss_communication_aprsmsg.d \
./src/kiss_protocol/kiss_did.d \
./src/kiss_protocol/kiss_nrc_response.d \
./src/kiss_protocol/kiss_security_access.d 


# Each subdirectory must supply rules for building sources it contributes
src/kiss_protocol/%.o: ../src/kiss_protocol/%.c src/kiss_protocol/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -DDEBUG -DHI_SPEED -DUSE_FULL_LL_DRIVER -DSTM32L471xx -DPARAMETEO -USTM32F10X_MD_VL -UPARATNC_HWREV_A -UPARATNC_HWREV_B -UPARATNC_HWREV_C -I"../include" -I"../include/configuration_nvm" -I"../include/etc" -I"../system/include/tiny-aes" -I"../system/include/aprs" -I"../system/include" -I"../system/include/cmsis/stm32l4xx" -I"../system/include/cmsis/stm32l4xx/device" -I"../system/include/stm32l4-hal-driver" -I"../system/include/stm32l4-hal-driver/Legacy" -std=gnu11 -Wunused-function -Wall -Wa,-adhlns="$@.lst" -fstack-usage -fdump-rtl-dfinish -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


