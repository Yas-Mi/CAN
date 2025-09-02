################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/mng/can_mng.c 

OBJS += \
./Core/Src/mng/can_mng.o 

C_DEPS += \
./Core/Src/mng/can_mng.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/mng/%.o Core/Src/mng/%.su Core/Src/mng/%.cyclo: ../Core/Src/mng/%.c Core/Src/mng/subdir.mk
<<<<<<< HEAD
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/app" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/dev" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/drv" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/mng" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/peri" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
=======
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I"C:/work/CAN/CAN/Core/Src/app" -I"C:/work/CAN/CAN/Core/Src/dev" -I"C:/work/CAN/CAN/Core/Src/drv" -I"C:/work/CAN/CAN/Core/Src/mng" -I"C:/work/CAN/CAN/Core/Src/peri" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
>>>>>>> 7b98383d0a153eba3c8ccfa25f6e730fb8e733de

clean: clean-Core-2f-Src-2f-mng

clean-Core-2f-Src-2f-mng:
	-$(RM) ./Core/Src/mng/can_mng.cyclo ./Core/Src/mng/can_mng.d ./Core/Src/mng/can_mng.o ./Core/Src/mng/can_mng.su

.PHONY: clean-Core-2f-Src-2f-mng

