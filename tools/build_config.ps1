# Project-specific firmware build configuration.
#
# Change this file when switching MCU families, linker scripts, startup files,
# HAL source sets, include paths, preprocessor macros, or application source
# files. tools/build.ps1 intentionally stays generic.

$BuildConfig = @{
    FirmwareName = "firmware"

    Defines = @(
        "STM32F407xx",
        "USE_HAL_DRIVER",
        "HSE_VALUE=8000000"
    )

    Includes = @(
        "include",
        "Drivers/STM32F4xx/CMSIS/Core/Include",
        "Drivers/STM32F4xx/CMSIS/Include",
        "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Include",
        "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Inc"
    )

    CpuFlags = @(
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfpu=fpv4-sp-d16",
        "-mfloat-abi=softfp"
    )

    SourcesC = @(
        "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c",
        "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c",
        "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c",
        "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c",
        "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c",
        "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c",
        "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    )

    SourcesCpp = @(
        "src/diagnostics.cpp",
        "src/frame.cpp",
        "src/scheduler.cpp",
        "src/task.cpp",
        "src/motor.cpp",
        "src/commands.cpp",
        "src/hal_hooks.cpp",
        "src/main.cpp"
    )

    SourcesAsm = @(
        "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f407xx.s"
    )

    LinkerScript = "linker/STM32F407ZE_FLASH.ld"

    ExtraCommonFlags = @()
    ExtraCFlags = @()
    ExtraCxxFlags = @()
    ExtraAsmFlags = @()
    ExtraLinkFlags = @()
}
