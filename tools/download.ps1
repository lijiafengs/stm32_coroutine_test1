param(
    [string]$Toolchain = "",
    [string]$JLinkGdbServer = "C:\Program Files (x86)\SEGGER\JLink_V490e\JLinkGDBServerCL.exe",
    [string]$Firmware = "",
    [string]$Device = "STM32F407ZE",
    [string]$Interface = "SWD",
    [string]$Speed = "2000",
    [int]$Port = 2331
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($Toolchain)) {
    if (![string]::IsNullOrWhiteSpace($env:ARM_GCC_PATH)) {
        $Toolchain = $env:ARM_GCC_PATH
    } else {
        $Toolchain = "C:\SysGCC\arm-eabi\bin"
    }
}

if ([string]::IsNullOrWhiteSpace($Firmware)) {
    $Firmware = Join-Path $Root "build\firmware.elf"
}

$Gdb = Join-Path $Toolchain "arm-none-eabi-gdb.exe"

if (!(Test-Path $Gdb)) {
    throw "Missing GDB: $Gdb"
}

if (!(Test-Path $Firmware)) {
    throw "Missing firmware ELF: $Firmware"
}

& (Join-Path $PSScriptRoot "start-jlink-gdbserver.ps1") `
    -JLinkGdbServer $JLinkGdbServer `
    -Device $Device `
    -Interface $Interface `
    -Speed $Speed `
    -Port $Port

$GdbCommands = @(
    "set confirm off",
    "set pagination off",
    "target remote localhost:$Port",
    "monitor reset",
    "monitor halt",
    "load",
    "monitor reset",
    "monitor go",
    "detach",
    "quit"
)

$GdbArgs = @(
    "--batch",
    "--quiet",
    $Firmware
)

foreach ($command in $GdbCommands) {
    $GdbArgs += @("-ex", $command)
}

& $Gdb @GdbArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Downloaded: $Firmware"
