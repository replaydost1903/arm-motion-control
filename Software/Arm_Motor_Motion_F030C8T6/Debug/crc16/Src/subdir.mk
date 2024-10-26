################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../crc16/Src/crc16.c 

OBJS += \
./crc16/Src/crc16.o 

C_DEPS += \
./crc16/Src/crc16.d 


# Each subdirectory must supply rules for building sources it contributes
crc16/Src/%.o crc16/Src/%.su crc16/Src/%.cyclo: ../crc16/Src/%.c crc16/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/crc16/Inc" -I"C:/ODETRON/Arm_Motor_Motion_F030C8T6/rs485/Inc" -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-crc16-2f-Src

clean-crc16-2f-Src:
	-$(RM) ./crc16/Src/crc16.cyclo ./crc16/Src/crc16.d ./crc16/Src/crc16.o ./crc16/Src/crc16.su

.PHONY: clean-crc16-2f-Src

