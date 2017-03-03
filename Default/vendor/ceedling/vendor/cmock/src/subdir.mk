################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/ceedling/vendor/cmock/src/cmock.c 

OBJS += \
./vendor/ceedling/vendor/cmock/src/cmock.o 

C_DEPS += \
./vendor/ceedling/vendor/cmock/src/cmock.d 


# Each subdirectory must supply rules for building sources it contributes
vendor/ceedling/vendor/cmock/src/%.o: ../vendor/ceedling/vendor/cmock/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


