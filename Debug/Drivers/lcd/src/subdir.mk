################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/lcd/src/lcd.c \
../Drivers/lcd/src/lcd_port.c 

OBJS += \
./Drivers/lcd/src/lcd.o \
./Drivers/lcd/src/lcd_port.o 

C_DEPS += \
./Drivers/lcd/src/lcd.d \
./Drivers/lcd/src/lcd_port.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/lcd/src/%.o Drivers/lcd/src/%.su Drivers/lcd/src/%.cyclo: ../Drivers/lcd/src/%.c Drivers/lcd/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/rc522" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/lcd" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/rc522/inc" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/API" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/API/Inc" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/lcd/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-lcd-2f-src

clean-Drivers-2f-lcd-2f-src:
	-$(RM) ./Drivers/lcd/src/lcd.cyclo ./Drivers/lcd/src/lcd.d ./Drivers/lcd/src/lcd.o ./Drivers/lcd/src/lcd.su ./Drivers/lcd/src/lcd_port.cyclo ./Drivers/lcd/src/lcd_port.d ./Drivers/lcd/src/lcd_port.o ./Drivers/lcd/src/lcd_port.su

.PHONY: clean-Drivers-2f-lcd-2f-src

