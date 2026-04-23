################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/rc522/src/rc522.c \
../Drivers/rc522/src/rc522_port.c 

OBJS += \
./Drivers/rc522/src/rc522.o \
./Drivers/rc522/src/rc522_port.o 

C_DEPS += \
./Drivers/rc522/src/rc522.d \
./Drivers/rc522/src/rc522_port.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/rc522/src/%.o Drivers/rc522/src/%.su Drivers/rc522/src/%.cyclo: ../Drivers/rc522/src/%.c Drivers/rc522/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/rc522" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/lcd" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/rc522/inc" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/API" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/API/Inc" -I"/home/andres/cese/pdm/rfid_card_reader/Drivers/lcd/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-rc522-2f-src

clean-Drivers-2f-rc522-2f-src:
	-$(RM) ./Drivers/rc522/src/rc522.cyclo ./Drivers/rc522/src/rc522.d ./Drivers/rc522/src/rc522.o ./Drivers/rc522/src/rc522.su ./Drivers/rc522/src/rc522_port.cyclo ./Drivers/rc522/src/rc522_port.d ./Drivers/rc522/src/rc522_port.o ./Drivers/rc522/src/rc522_port.su

.PHONY: clean-Drivers-2f-rc522-2f-src

