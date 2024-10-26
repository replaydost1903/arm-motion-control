################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../appdriver/rs485/Src/rs485.c 

OBJS += \
./appdriver/rs485/Src/rs485.o 

C_DEPS += \
./appdriver/rs485/Src/rs485.d 


# Each subdirectory must supply rules for building sources it contributes
appdriver/rs485/Src/%.o appdriver/rs485/Src/%.su appdriver/rs485/Src/%.cyclo: ../appdriver/rs485/Src/%.c appdriver/rs485/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/Inc" -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/crc16/Inc" -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/flash/Inc" -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/appdriver/rs485/Inc" -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-appdriver-2f-rs485-2f-Src

clean-appdriver-2f-rs485-2f-Src:
	-$(RM) ./appdriver/rs485/Src/rs485.cyclo ./appdriver/rs485/Src/rs485.d ./appdriver/rs485/Src/rs485.o ./appdriver/rs485/Src/rs485.su

.PHONY: clean-appdriver-2f-rs485-2f-Src

