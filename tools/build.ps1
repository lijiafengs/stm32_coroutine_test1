param(
    [string]$Toolchain = ""
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"
$ObjDir = Join-Path $BuildDir "obj"

if ([string]::IsNullOrWhiteSpace($Toolchain)) {
    if (![string]::IsNullOrWhiteSpace($env:ARM_GCC_PATH)) {
        $Toolchain = $env:ARM_GCC_PATH
    } else {
        $Toolchain = "C:\SysGCC\arm-eabi\bin"
    }
}

$CC = Join-Path $Toolchain "arm-none-eabi-gcc.exe"
$CXX = Join-Path $Toolchain "arm-none-eabi-g++.exe"
$Objcopy = Join-Path $Toolchain "arm-none-eabi-objcopy.exe"
$Size = Join-Path $Toolchain "arm-none-eabi-size.exe"

foreach ($tool in @($CC, $CXX, $Objcopy, $Size)) {
    if (!(Test-Path $tool)) {
        throw "Missing tool: $tool"
    }
}

New-Item -ItemType Directory -Force -Path $BuildDir, $ObjDir | Out-Null

$Defines = @(
    "-DSTM32F407xx",
    "-DUSE_HAL_DRIVER",
    "-DHSE_VALUE=8000000"
)

$Includes = @(
    "include",
    "Drivers/STM32F4xx/CMSIS/Core/Include",
    "Drivers/STM32F4xx/CMSIS/Include",
    "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Include",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Inc"
) | ForEach-Object { "-I$(Join-Path $Root $_)" }

$CpuFlags = @(
    "-mcpu=cortex-m4",
    "-mthumb",
    "-mfpu=fpv4-sp-d16",
    "-mfloat-abi=softfp"
)

$CommonFlags = @(
    "-g3",
    "-Og",
    "-ffunction-sections",
    "-fdata-sections",
    "-Wall",
    "-Wextra",
    "-Wno-unused-parameter"
) + $CpuFlags + $Defines + $Includes

$CFlags = @("-std=gnu11") + $CommonFlags
$CxxFlags = @(
    "-std=gnu++20",
    "-fno-exceptions",
    "-fno-rtti",
    "-fno-use-cxa-atexit"
) + $CommonFlags

$AsmFlags = @(
    "-x", "assembler-with-cpp"
) + $CommonFlags

$SourcesC = @(
    "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
)

$SourcesCpp = @(
    "src/diagnostics.cpp",
    "src/frame.cpp",
    "src/scheduler.cpp",
    "src/task.cpp",
    "src/motor.cpp",
    "src/commands.cpp",
    "src/hal_hooks.cpp",
    "src/main.cpp"
)

$SourcesAsm = @(
    "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f407xx.s"
)

function Get-ObjectPath($RelativePath) {
    $safe = $RelativePath -replace '[:\\/\.]', '_'
    return Join-Path $ObjDir "$safe.o"
}

$Objects = @()

foreach ($src in $SourcesC) {
    $obj = Get-ObjectPath $src
    $Objects += $obj
    & $CC @CFlags -c (Join-Path $Root $src) -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

foreach ($src in $SourcesCpp) {
    $obj = Get-ObjectPath $src
    $Objects += $obj
    & $CXX @CxxFlags -c (Join-Path $Root $src) -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

foreach ($src in $SourcesAsm) {
    $obj = Get-ObjectPath $src
    $Objects += $obj
    & $CC @AsmFlags -c (Join-Path $Root $src) -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$Elf = Join-Path $BuildDir "firmware.elf"
$Hex = Join-Path $BuildDir "firmware.hex"
$Bin = Join-Path $BuildDir "firmware.bin"
$Map = Join-Path $BuildDir "firmware.map"
$LinkerScript = Join-Path $Root "linker/STM32F407ZE_FLASH.ld"

$LinkFlags = @(
    "-T$LinkerScript",
    "-Wl,-Map=$Map",
    "-Wl,--gc-sections",
    "-Wl,--print-memory-usage",
    "--specs=nano.specs",
    "--specs=nosys.specs"
) + $CpuFlags

& $CXX @Objects @LinkFlags -o $Elf
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Objcopy -O ihex $Elf $Hex
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Objcopy -O binary $Elf $Bin
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Size $Elf
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Built: $Elf"
Write-Host "Built: $Hex"
Write-Host "Built: $Bin"
