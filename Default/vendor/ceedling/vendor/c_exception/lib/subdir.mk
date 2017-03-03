################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/ceedling/vendor/c_exception/lib/CException.c 

OBJS += \
./vendor/ceedling/vendor/c_exception/lib/CException.o 

C_DEPS += \
./vendor/ceedling/vendor/c_exception/lib/CException.d 


# Each subdirectory must supply rules for building sources it contributes
vendor/ceedling/vendor/c_exception/lib/%.o: ../vendor/ceedling/vendor/c_exception/lib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


