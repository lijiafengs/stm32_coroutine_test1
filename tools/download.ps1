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

. (Join-Path $PSScriptRoot "vscode-settings.ps1")
. (Join-Path $PSScriptRoot "jlink_config.ps1")

$Toolchain = Resolve-ToolchainPath $Toolchain
$JLinkGdbServer = Resolve-JLinkGdbServerPath $JLinkGdbServer
$Device = Resolve-JLinkStringValue $Device "Device"
$Interface = Resolve-JLinkStringValue $Interface "Interface"
$Speed = Resolve-JLinkStringValue $Speed "Speed"
$Port = Resolve-JLinkIntValue $Port "Port"

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
