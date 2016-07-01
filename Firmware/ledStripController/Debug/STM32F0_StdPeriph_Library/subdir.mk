################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../STM32F0_StdPeriph_Library/stm32f0xx_adc.c \
../STM32F0_StdPeriph_Library/stm32f0xx_can.c \
../STM32F0_StdPeriph_Library/stm32f0xx_cec.c \
../STM32F0_StdPeriph_Library/stm32f0xx_comp.c \
../STM32F0_StdPeriph_Library/stm32f0xx_crc.c \
../STM32F0_StdPeriph_Library/stm32f0xx_crs.c \
../STM32F0_StdPeriph_Library/stm32f0xx_dac.c \
../STM32F0_StdPeriph_Library/stm32f0xx_dbgmcu.c \
../STM32F0_StdPeriph_Library/stm32f0xx_dma.c \
../STM32F0_StdPeriph_Library/stm32f0xx_exti.c \
../STM32F0_StdPeriph_Library/stm32f0xx_flash.c \
../STM32F0_StdPeriph_Library/stm32f0xx_gpio.c \
../STM32F0_StdPeriph_Library/stm32f0xx_i2c.c \
../STM32F0_StdPeriph_Library/stm32f0xx_iwdg.c \
../STM32F0_StdPeriph_Library/stm32f0xx_misc.c \
../STM32F0_StdPeriph_Library/stm32f0xx_pwr.c \
../STM32F0_StdPeriph_Library/stm32f0xx_rcc.c \
../STM32F0_StdPeriph_Library/stm32f0xx_rtc.c \
../STM32F0_StdPeriph_Library/stm32f0xx_spi.c \
../STM32F0_StdPeriph_Library/stm32f0xx_syscfg.c \
../STM32F0_StdPeriph_Library/stm32f0xx_tim.c \
../STM32F0_StdPeriph_Library/stm32f0xx_usart.c \
../STM32F0_StdPeriph_Library/stm32f0xx_wwdg.c 

OBJS += \
./STM32F0_StdPeriph_Library/stm32f0xx_adc.o \
./STM32F0_StdPeriph_Library/stm32f0xx_can.o \
./STM32F0_StdPeriph_Library/stm32f0xx_cec.o \
./STM32F0_StdPeriph_Library/stm32f0xx_comp.o \
./STM32F0_StdPeriph_Library/stm32f0xx_crc.o \
./STM32F0_StdPeriph_Library/stm32f0xx_crs.o \
./STM32F0_StdPeriph_Library/stm32f0xx_dac.o \
./STM32F0_StdPeriph_Library/stm32f0xx_dbgmcu.o \
./STM32F0_StdPeriph_Library/stm32f0xx_dma.o \
./STM32F0_StdPeriph_Library/stm32f0xx_exti.o \
./STM32F0_StdPeriph_Library/stm32f0xx_flash.o \
./STM32F0_StdPeriph_Library/stm32f0xx_gpio.o \
./STM32F0_StdPeriph_Library/stm32f0xx_i2c.o \
./STM32F0_StdPeriph_Library/stm32f0xx_iwdg.o \
./STM32F0_StdPeriph_Library/stm32f0xx_misc.o \
./STM32F0_StdPeriph_Library/stm32f0xx_pwr.o \
./STM32F0_StdPeriph_Library/stm32f0xx_rcc.o \
./STM32F0_StdPeriph_Library/stm32f0xx_rtc.o \
./STM32F0_StdPeriph_Library/stm32f0xx_spi.o \
./STM32F0_StdPeriph_Library/stm32f0xx_syscfg.o \
./STM32F0_StdPeriph_Library/stm32f0xx_tim.o \
./STM32F0_StdPeriph_Library/stm32f0xx_usart.o \
./STM32F0_StdPeriph_Library/stm32f0xx_wwdg.o 

C_DEPS += \
./STM32F0_StdPeriph_Library/stm32f0xx_adc.d \
./STM32F0_StdPeriph_Library/stm32f0xx_can.d \
./STM32F0_StdPeriph_Library/stm32f0xx_cec.d \
./STM32F0_StdPeriph_Library/stm32f0xx_comp.d \
./STM32F0_StdPeriph_Library/stm32f0xx_crc.d \
./STM32F0_StdPeriph_Library/stm32f0xx_crs.d \
./STM32F0_StdPeriph_Library/stm32f0xx_dac.d \
./STM32F0_StdPeriph_Library/stm32f0xx_dbgmcu.d \
./STM32F0_StdPeriph_Library/stm32f0xx_dma.d \
./STM32F0_StdPeriph_Library/stm32f0xx_exti.d \
./STM32F0_StdPeriph_Library/stm32f0xx_flash.d \
./STM32F0_StdPeriph_Library/stm32f0xx_gpio.d \
./STM32F0_StdPeriph_Library/stm32f0xx_i2c.d \
./STM32F0_StdPeriph_Library/stm32f0xx_iwdg.d \
./STM32F0_StdPeriph_Library/stm32f0xx_misc.d \
./STM32F0_StdPeriph_Library/stm32f0xx_pwr.d \
./STM32F0_StdPeriph_Library/stm32f0xx_rcc.d \
./STM32F0_StdPeriph_Library/stm32f0xx_rtc.d \
./STM32F0_StdPeriph_Library/stm32f0xx_spi.d \
./STM32F0_StdPeriph_Library/stm32f0xx_syscfg.d \
./STM32F0_StdPeriph_Library/stm32f0xx_tim.d \
./STM32F0_StdPeriph_Library/stm32f0xx_usart.d \
./STM32F0_StdPeriph_Library/stm32f0xx_wwdg.d 


# Each subdirectory must supply rules for building sources it contributes
STM32F0_StdPeriph_Library/%.o: ../STM32F0_StdPeriph_Library/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


