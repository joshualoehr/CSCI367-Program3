################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/prog3_observer.c \
../src/prog3_participant.c \
../src/prog3_server.c 

OBJS += \
./src/prog3_observer.o \
./src/prog3_participant.o \
./src/prog3_server.o 

C_DEPS += \
./src/prog3_observer.d \
./src/prog3_participant.d \
./src/prog3_server.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


