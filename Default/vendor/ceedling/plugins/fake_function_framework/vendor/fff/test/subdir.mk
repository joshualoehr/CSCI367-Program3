################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/fff_test_c.c \
../vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/fff_test_global_c.c \
../vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/global_fakes.c 

OBJS += \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/fff_test_c.o \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/fff_test_global_c.o \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/global_fakes.o 

C_DEPS += \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/fff_test_c.d \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/fff_test_global_c.d \
./vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/global_fakes.d 


# Each subdirectory must supply rules for building sources it contributes
vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/%.o: ../vendor/ceedling/plugins/fake_function_framework/vendor/fff/test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


