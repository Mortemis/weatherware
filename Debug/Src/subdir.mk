################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/arp.c \
../Src/enc28j60.c \
../Src/freertos.c \
../Src/main.c \
../Src/net.c \
../Src/stm32f1xx_hal_msp.c \
../Src/stm32f1xx_hal_timebase_tim.c \
../Src/stm32f1xx_it.c \
../Src/syscalls.c \
../Src/system_stm32f1xx.c \
../Src/udp.c 

OBJS += \
./Src/arp.o \
./Src/enc28j60.o \
./Src/freertos.o \
./Src/main.o \
./Src/net.o \
./Src/stm32f1xx_hal_msp.o \
./Src/stm32f1xx_hal_timebase_tim.o \
./Src/stm32f1xx_it.o \
./Src/syscalls.o \
./Src/system_stm32f1xx.o \
./Src/udp.o 

C_DEPS += \
./Src/arp.d \
./Src/enc28j60.d \
./Src/freertos.d \
./Src/main.d \
./Src/net.d \
./Src/stm32f1xx_hal_msp.d \
./Src/stm32f1xx_hal_timebase_tim.d \
./Src/stm32f1xx_it.d \
./Src/syscalls.d \
./Src/system_stm32f1xx.d \
./Src/udp.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F103xB -I"D:/embed/weatherware/Inc" -I"D:/embed/weatherware/Drivers/STM32F1xx_HAL_Driver/Inc" -I"D:/embed/weatherware/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"D:/embed/weatherware/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"D:/embed/weatherware/Drivers/CMSIS/Include" -I"D:/embed/weatherware/Middlewares/Third_Party/FreeRTOS/Source/include" -I"D:/embed/weatherware/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"D:/embed/weatherware/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


