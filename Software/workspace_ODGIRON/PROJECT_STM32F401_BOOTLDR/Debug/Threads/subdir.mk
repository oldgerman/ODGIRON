################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
D:/ST/workspace_ODGIRON/Threads/GUIThread.cpp \
D:/ST/workspace_ODGIRON/Threads/MOVThread.cpp \
D:/ST/workspace_ODGIRON/Threads/PIDThread.cpp \
D:/ST/workspace_ODGIRON/Threads/POWThread.cpp \
D:/ST/workspace_ODGIRON/Threads/USBThread.cpp 

OBJS += \
./Threads/GUIThread.o \
./Threads/MOVThread.o \
./Threads/PIDThread.o \
./Threads/POWThread.o \
./Threads/USBThread.o 

CPP_DEPS += \
./Threads/GUIThread.d \
./Threads/MOVThread.d \
./Threads/PIDThread.d \
./Threads/POWThread.d \
./Threads/USBThread.d 


# Each subdirectory must supply rules for building sources it contributes
Threads/GUIThread.o: D:/ST/workspace_ODGIRON/Threads/GUIThread.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/ST/workspace_ODGIRON/BSP/Inc" -I"D:/ST/workspace_ODGIRON/Fonts" -I"D:/ST/workspace_ODGIRON/HARDWARE/Inc" -I"D:/ST/workspace_ODGIRON/HARDWARE/FUSB302" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src/clib" -I"D:/ST/workspace_ODGIRON/SOFTWARE/Inc" -I"D:/ST/workspace_ODGIRON/Threads" -Og -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Threads/GUIThread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Threads/MOVThread.o: D:/ST/workspace_ODGIRON/Threads/MOVThread.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/ST/workspace_ODGIRON/BSP/Inc" -I"D:/ST/workspace_ODGIRON/Fonts" -I"D:/ST/workspace_ODGIRON/HARDWARE/Inc" -I"D:/ST/workspace_ODGIRON/HARDWARE/FUSB302" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src/clib" -I"D:/ST/workspace_ODGIRON/SOFTWARE/Inc" -I"D:/ST/workspace_ODGIRON/Threads" -Og -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Threads/MOVThread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Threads/PIDThread.o: D:/ST/workspace_ODGIRON/Threads/PIDThread.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/ST/workspace_ODGIRON/BSP/Inc" -I"D:/ST/workspace_ODGIRON/Fonts" -I"D:/ST/workspace_ODGIRON/HARDWARE/Inc" -I"D:/ST/workspace_ODGIRON/HARDWARE/FUSB302" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src/clib" -I"D:/ST/workspace_ODGIRON/SOFTWARE/Inc" -I"D:/ST/workspace_ODGIRON/Threads" -Og -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Threads/PIDThread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Threads/POWThread.o: D:/ST/workspace_ODGIRON/Threads/POWThread.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/ST/workspace_ODGIRON/BSP/Inc" -I"D:/ST/workspace_ODGIRON/Fonts" -I"D:/ST/workspace_ODGIRON/HARDWARE/Inc" -I"D:/ST/workspace_ODGIRON/HARDWARE/FUSB302" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src/clib" -I"D:/ST/workspace_ODGIRON/SOFTWARE/Inc" -I"D:/ST/workspace_ODGIRON/Threads" -Og -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Threads/POWThread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Threads/USBThread.o: D:/ST/workspace_ODGIRON/Threads/USBThread.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/ST/workspace_ODGIRON/BSP/Inc" -I"D:/ST/workspace_ODGIRON/Fonts" -I"D:/ST/workspace_ODGIRON/HARDWARE/Inc" -I"D:/ST/workspace_ODGIRON/HARDWARE/FUSB302" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src" -I"D:/ST/workspace_ODGIRON/HARDWARE/u8g2/src/clib" -I"D:/ST/workspace_ODGIRON/SOFTWARE/Inc" -I"D:/ST/workspace_ODGIRON/Threads" -Og -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"Threads/USBThread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

