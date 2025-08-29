################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/dev/btn_dev.c \
../Core/Src/dev/mcp2515.c 

OBJS += \
./Core/Src/dev/btn_dev.o \
./Core/Src/dev/mcp2515.o 

C_DEPS += \
./Core/Src/dev/btn_dev.d \
./Core/Src/dev/mcp2515.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/dev/%.o Core/Src/dev/%.su Core/Src/dev/%.cyclo: ../Core/Src/dev/%.c Core/Src/dev/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I"C:/work/CAN/CAN/Core/Src/app" -I"C:/work/CAN/CAN/Core/Src/dev" -I"C:/work/CAN/CAN/Core/Src/drv" -I"C:/work/CAN/CAN/Core/Src/mng" -I"C:/work/CAN/CAN/Core/Src/peri" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-dev

clean-Core-2f-Src-2f-dev:
	-$(RM) ./Core/Src/dev/btn_dev.cyclo ./Core/Src/dev/btn_dev.d ./Core/Src/dev/btn_dev.o ./Core/Src/dev/btn_dev.su ./Core/Src/dev/mcp2515.cyclo ./Core/Src/dev/mcp2515.d ./Core/Src/dev/mcp2515.o ./Core/Src/dev/mcp2515.su

.PHONY: clean-Core-2f-Src-2f-dev

