################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/drv/can_drv.c \
../Core/Src/drv/usart_drv.c 

OBJS += \
./Core/Src/drv/can_drv.o \
./Core/Src/drv/usart_drv.o 

C_DEPS += \
./Core/Src/drv/can_drv.d \
./Core/Src/drv/usart_drv.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/drv/%.o Core/Src/drv/%.su Core/Src/drv/%.cyclo: ../Core/Src/drv/%.c Core/Src/drv/subdir.mk
<<<<<<< HEAD
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/app" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/dev" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/drv" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/mng" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/peri" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
=======
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I"C:/work/CAN/CAN/Core/Src/app" -I"C:/work/CAN/CAN/Core/Src/dev" -I"C:/work/CAN/CAN/Core/Src/drv" -I"C:/work/CAN/CAN/Core/Src/mng" -I"C:/work/CAN/CAN/Core/Src/peri" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
>>>>>>> 7b98383d0a153eba3c8ccfa25f6e730fb8e733de

clean: clean-Core-2f-Src-2f-drv

clean-Core-2f-Src-2f-drv:
	-$(RM) ./Core/Src/drv/can_drv.cyclo ./Core/Src/drv/can_drv.d ./Core/Src/drv/can_drv.o ./Core/Src/drv/can_drv.su ./Core/Src/drv/usart_drv.cyclo ./Core/Src/drv/usart_drv.d ./Core/Src/drv/usart_drv.o ./Core/Src/drv/usart_drv.su

.PHONY: clean-Core-2f-Src-2f-drv

