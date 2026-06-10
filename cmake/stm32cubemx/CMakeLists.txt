cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	USE_NUCLEO_64 
	CORE_CM4 
	USE_HAL_DRIVER 
	STM32WL55xx
    $<$<CONFIG:Debug>:DEBUG>
)

set(FREERTOS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS)

# STM32CubeMX generated inclde paths
set(MX_Include_Dirs
   ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/freertos
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
     ${CMAKE_CURRENT_SOURCE_DIR}/../../App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SubGHz_Phy/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SubGHz_Phy/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/trace/adv_trace
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/misc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/timer
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/lpm/tiny_lpm
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/STM32WLxx_Nucleo
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32WLxx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SubGHz_Phy/radio_driver
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM3
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SubGHz_Phy/Target/radio_board_if.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SubGHz_Phy/App/app_subghz_phy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SubGHz_Phy/App/subghz_phy_app.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/WdtSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/subghz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32_lpm_if.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/sys_app.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/timer_if.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32_adv_trace_if.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32wlxx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32wlxx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32wl55xx_cm4.s
)

# my application 
set(MyAplication_src
     ${CMAKE_CURRENT_SOURCE_DIR}/../../App/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/DataTransmit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/Packet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/PumpControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/DisplayModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/ButtonControl.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../../App/LedDriver.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/../../App/FlashStorage.cpp
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_stm32wlxx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_adc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_ll_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_crc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rtc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_subghz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_usart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/STM32WLxx_Nucleo/stm32wlxx_nucleo.c
)

# Drivers Midllewares
set(Utilities_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/trace/adv_trace/stm32_adv_trace.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/misc/stm32_mem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/misc/stm32_systime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/misc/stm32_tiny_sscanf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/misc/stm32_tiny_vsnprintf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/timer/stm32_timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/lpm/tiny_lpm/stm32_lpm.c
)

set(FreeRTOS_Src
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_DIR}/portable/GCC/ARM_CM3/port.c
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

set(SubGHz_Phy_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SubGHz_Phy/radio_driver/lr_fhss_mac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SubGHz_Phy/radio_driver/radio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SubGHz_Phy/radio_driver/radio_driver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SubGHz_Phy/radio_driver/radio_fw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SubGHz_Phy/radio_driver/wl_lr_fhss.c
)

# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    Utilities
	FreeRTOS
	SubGHz_Phy
    MyApp
   ${TOOLCHAIN_LINK_LIBRARIES}
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx SYSTEM INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)


# Create Utilities static library
add_library(Utilities OBJECT)
target_sources(Utilities PRIVATE ${Utilities_Src})
target_link_libraries(Utilities PUBLIC stm32cubemx)

# Create FreeRTOS static library
add_library(FreeRTOS OBJECT)
target_sources(FreeRTOS PRIVATE ${FreeRTOS_Src})
target_link_libraries(FreeRTOS PUBLIC stm32cubemx)

# Create SubGHz_Phy static library
add_library(SubGHz_Phy OBJECT)
target_sources(SubGHz_Phy PRIVATE ${SubGHz_Phy_Src})
target_link_libraries(SubGHz_Phy PUBLIC stm32cubemx)


#create my utilities lib
add_library(MyApp OBJECT) 
target_sources(MyApp PRIVATE ${MyAplication_src})
target_link_libraries(MyApp PUBLIC stm32cubemx)

# Add STM32CubeMX generated application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
