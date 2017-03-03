################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/UI.c \
../vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/UI_test_ansic.c \
../vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/test_suite_template.c 

OBJS += \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/UI.o \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/UI_test_ansic.o \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/test_suite_template.o 

C_DEPS += \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/UI.d \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/UI_test_ansic.d \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/test_suite_template.d 


# Each subdirectory must supply rules for building sources it contributes
vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/%.o: ../vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/embedded_ui/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


