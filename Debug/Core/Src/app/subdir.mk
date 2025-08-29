################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app/can_app.c \
../Core/Src/app/console.c \
../Core/Src/app/standby_app.c 

OBJS += \
./Core/Src/app/can_app.o \
./Core/Src/app/console.o \
./Core/Src/app/standby_app.o 

C_DEPS += \
./Core/Src/app/can_app.d \
./Core/Src/app/console.d \
./Core/Src/app/standby_app.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/app/%.o Core/Src/app/%.su Core/Src/app/%.cyclo: ../Core/Src/app/%.c Core/Src/app/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F769xx -c -I../Core/Inc -I"C:/work/CAN/CAN/Core/Src/app" -I"C:/work/CAN/CAN/Core/Src/dev" -I"C:/work/CAN/CAN/Core/Src/drv" -I"C:/work/CAN/CAN/Core/Src/mng" -I"C:/work/CAN/CAN/Core/Src/peri" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-app

clean-Core-2f-Src-2f-app:
	-$(RM) ./Core/Src/app/can_app.cyclo ./Core/Src/app/can_app.d ./Core/Src/app/can_app.o ./Core/Src/app/can_app.su ./Core/Src/app/console.cyclo ./Core/Src/app/console.d ./Core/Src/app/console.o ./Core/Src/app/console.su ./Core/Src/app/standby_app.cyclo ./Core/Src/app/standby_app.d ./Core/Src/app/standby_app.o ./Core/Src/app/standby_app.su

.PHONY: clean-Core-2f-Src-2f-app

