################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../prog3_observer.c \
../prog3_participant.c \
../prog3_server.c 

OBJS += \
./prog3_observer.o \
./prog3_participant.o \
./prog3_server.o 

C_DEPS += \
./prog3_observer.d \
./prog3_participant.d \
./prog3_server.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


