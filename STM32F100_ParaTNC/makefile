################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include system/src/ve_direct_protocol/subdir.mk
-include system/src/umb_master/subdir.mk
-include system/src/tm/subdir.mk
-include system/src/tiny-aes/subdir.mk
-include system/src/stm32f1-stdperiph/subdir.mk
-include system/src/skytrax_fanet/subdir.mk
-include system/src/newlib/subdir.mk
-include system/src/modbus_rtu/subdir.mk
-include system/src/http_client/subdir.mk
-include system/src/dust_sensor/subdir.mk
-include system/src/drivers/sx1262/subdir.mk
-include system/src/drivers/f1/subdir.mk
-include system/src/drivers/subdir.mk
-include system/src/diag/subdir.mk
-include system/src/davis_vantage/subdir.mk
-include system/src/cortexm/subdir.mk
-include system/src/cmsis/subdir.mk
-include system/src/aprs/subdir.mk
-include system/src/subdir.mk
-include src/stored_configuration_nvm/subdir.mk
-include src/nvm/subdir.mk
-include src/kiss_protocol/diagnostics_services/subdir.mk
-include src/kiss_protocol/diagnostics_routines/subdir.mk
-include src/kiss_protocol/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ParaTNC
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
ParaTNC.hex \

SECONDARY_SIZE += \
ParaTNC.siz \


# All Target
all: main-build

# Main-build Target
main-build: ParaTNC.elf secondary-outputs

# Tool invocations
ParaTNC.elf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -g3 -T mem.ld -T libs.ld -T sections.ld -nostartfiles -Xlinker --gc-sections -L"../ldscripts/stm32f100" -Wl,-Map,"ParaTNC.map" --specs=nano.specs -u _printf_float -o "ParaTNC.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

ParaTNC.hex: ParaTNC.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "ParaTNC.elf"  "ParaTNC.hex"
	@echo 'Finished building: $@'
	@echo ' '

ParaTNC.siz: ParaTNC.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "ParaTNC.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS)$(CPP_DEPS) ParaTNC.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build

-include ../makefile.targets
