################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include vendor/ceedling/vendor/unity/src/subdir.mk
-include vendor/ceedling/vendor/cmock/src/subdir.mk
-include vendor/ceedling/vendor/c_exception/lib/subdir.mk
-include vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/subdir.mk
-include vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/subdir.mk
-include vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/driver_testing/subdir.mk
-include test/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: prog3

# Tool invocations
prog3: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "prog3" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) prog3
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
