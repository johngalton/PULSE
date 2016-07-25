################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/debug.c \
../src/led_driver.c \
../src/main.c \
../src/stm32l0xx_it.c \
../src/system_stm32l0xx.c \
../src/timer.c \
../src/uart.c 

OBJS += \
./src/debug.o \
./src/led_driver.o \
./src/main.o \
./src/stm32l0xx_it.o \
./src/system_stm32l0xx.o \
./src/timer.o \
./src/uart.o 

C_DEPS += \
./src/debug.d \
./src/led_driver.d \
./src/main.d \
./src/stm32l0xx_it.d \
./src/system_stm32l0xx.d \
./src/timer.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -std=c99 -DSTM32L052xx -DDEV_ID=2 -DUSE_STDPERIPH_DRIVER -I"C:\Users\John\Documents\GitHub\PULSE\Firmware\Drivers\CMSIS\Include" -I"C:\Users\John\Documents\GitHub\PULSE\Firmware\ledStripController\inc" -I"C:\Users\John\Documents\GitHub\PULSE\Firmware\Drivers\CMSIS\Device\ST\STM32L0xx\Include" -I"C:\Users\John\Documents\GitHub\PULSE\Firmware\Drivers\STM32L0xx_HAL_Driver\Inc" -O2 -g -Wall -c -fmessage-length=0 -mthumb -mcpu=cortex-m0plus -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


