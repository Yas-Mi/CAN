################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f769nihx.s 

OBJS += \
./Core/Startup/startup_stm32f769nihx.o 

S_DEPS += \
./Core/Startup/startup_stm32f769nihx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
<<<<<<< HEAD
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -DDEBUG -c -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/app" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/dev" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/drv" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/mng" -I"C:/Users/user/OneDrive/Desktop/CAN/CAN/Core/Src/peri" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"
=======
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -DDEBUG -c -I"C:/work/CAN/CAN/Core/Src/app" -I"C:/work/CAN/CAN/Core/Src/dev" -I"C:/work/CAN/CAN/Core/Src/drv" -I"C:/work/CAN/CAN/Core/Src/mng" -I"C:/work/CAN/CAN/Core/Src/peri" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"
>>>>>>> 7b98383d0a153eba3c8ccfa25f6e730fb8e733de

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f769nihx.d ./Core/Startup/startup_stm32f769nihx.o

.PHONY: clean-Core-2f-Startup

