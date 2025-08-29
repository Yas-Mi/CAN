################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/peri/can.c \
../Core/Src/peri/spi.c \
../Core/Src/peri/usart.c 

OBJS += \
./Core/Src/peri/can.o \
./Core/Src/peri/spi.o \
./Core/Src/peri/usart.o 

C_DEPS += \
./Core/Src/peri/can.d \
./Core/Src/peri/spi.d \
./Core/Src/peri/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/peri/%.o Core/Src/peri/%.su Core/Src/peri/%.cyclo: ../Core/Src/peri/%.c Core/Src/peri/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I"C:/work/CAN/CAN/Core/Src/app" -I"C:/work/CAN/CAN/Core/Src/dev" -I"C:/work/CAN/CAN/Core/Src/drv" -I"C:/work/CAN/CAN/Core/Src/mng" -I"C:/work/CAN/CAN/Core/Src/peri" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-peri

clean-Core-2f-Src-2f-peri:
	-$(RM) ./Core/Src/peri/can.cyclo ./Core/Src/peri/can.d ./Core/Src/peri/can.o ./Core/Src/peri/can.su ./Core/Src/peri/spi.cyclo ./Core/Src/peri/spi.d ./Core/Src/peri/spi.o ./Core/Src/peri/spi.su ./Core/Src/peri/usart.cyclo ./Core/Src/peri/usart.d ./Core/Src/peri/usart.o ./Core/Src/peri/usart.su

.PHONY: clean-Core-2f-Src-2f-peri

