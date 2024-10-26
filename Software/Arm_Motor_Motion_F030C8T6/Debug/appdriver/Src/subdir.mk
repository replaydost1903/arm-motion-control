################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../appdriver/Src/main_app.c 

OBJS += \
./appdriver/Src/main_app.o 

C_DEPS += \
./appdriver/Src/main_app.d 


# Each subdirectory must supply rules for building sources it contributes
appdriver/Src/%.o appdriver/Src/%.su appdriver/Src/%.cyclo: ../appdriver/Src/%.c appdriver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/Inc" -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/crc16/Inc" -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/flash/Inc" -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/rs485/Inc" -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-appdriver-2f-Src

clean-appdriver-2f-Src:
	-$(RM) ./appdriver/Src/main_app.cyclo ./appdriver/Src/main_app.d ./appdriver/Src/main_app.o ./appdriver/Src/main_app.su

.PHONY: clean-appdriver-2f-Src

