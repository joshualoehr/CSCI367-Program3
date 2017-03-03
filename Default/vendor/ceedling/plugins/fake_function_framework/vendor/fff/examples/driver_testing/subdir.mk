################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/driver_testing/driver.c 

OBJS += \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/driver_testing/driver.o 

C_DEPS += \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/driver_testing/driver.d 


# Each subdirectory must supply rules for building sources it contributes
vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/driver_testing/%.o: ../vendor/ceedling/plugins/fake_function_framework/vendor/fff/examples/driver_testing/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


