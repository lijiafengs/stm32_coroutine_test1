param(
    [string]$Toolchain = "",
    [string]$JLinkGdbServer = "",
    [string]$Firmware = "",
    [string]$Device = "",
    [string]$Interface = "",
    [string]$Speed = "",
    [int]$Port = 0
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot

$Config = Join-Path $PSScriptRoot "build_config.ps1"
if (Test-Path $Config) {
    . $Config
}

function Get-VSCodeSetting($Name) {
    $settingsPath = Join-Path $Root ".vscode\settings.json"
    if (!(Test-Path $settingsPath)) {
        return ""
    }

    $settings = Get-Content -Raw -Path $settingsPath | ConvertFrom-Json
    $property = $settings.PSObject.Properties[$Name]
    if ($null -eq $property) {
        return ""
    }
    return [string]$property.Value
}

if ([string]::IsNullOrWhiteSpace($Toolchain)) {
    if (![string]::IsNullOrWhiteSpace($env:ARM_GCC_PATH)) {
        $Toolchain = $env:ARM_GCC_PATH
    } else {
        $Toolchain = Get-VSCodeSetting "stm32.gccPath"
    }
}

if ([string]::IsNullOrWhiteSpace($JLinkGdbServer)) {
    $JLinkGdbServer = Get-VSCodeSetting "stm32.jlinkGdbServer"
}

if ([string]::IsNullOrWhiteSpace($Toolchain)) {
    throw "Missing GCC toolchain path. Set stm32.gccPath in .vscode/settings.json or pass -Toolchain."
}

if ([string]::IsNullOrWhiteSpace($JLinkGdbServer)) {
    throw "Missing J-Link GDB Server path. Set stm32.jlinkGdbServer in .vscode/settings.json or pass -JLinkGdbServer."
}

if ([string]::IsNullOrWhiteSpace($Firmware)) {
    $FirmwareName = if ($null -ne $BuildConfig -and $BuildConfig.ContainsKey("FirmwareName")) { $BuildConfig["FirmwareName"] } else { "firmware" }
    $Firmware = Join-Path $Root "build\$FirmwareName.elf"
}

if ([string]::IsNullOrWhiteSpace($Device)) {
    $Device = if ($null -ne $BuildConfig -and $BuildConfig.ContainsKey("JLinkDevice")) { $BuildConfig["JLinkDevice"] } else { "STM32F407ZE" }
}

if ([string]::IsNullOrWhiteSpace($Interface)) {
    $Interface = if ($null -ne $BuildConfig -and $BuildConfig.ContainsKey("JLinkInterface")) { $BuildConfig["JLinkInterface"] } else { "SWD" }
}

if ([string]::IsNullOrWhiteSpace($Speed)) {
    $Speed = if ($null -ne $BuildConfig -and $BuildConfig.ContainsKey("JLinkSpeed")) { $BuildConfig["JLinkSpeed"] } else { "2000" }
}

if ($Port -eq 0) {
    $Port = if ($null -ne $BuildConfig -and $BuildConfig.ContainsKey("JLinkPort")) { [int]$BuildConfig["JLinkPort"] } else { 2331 }
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
