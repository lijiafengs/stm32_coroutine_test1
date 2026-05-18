$ProjectRoot = Split-Path -Parent $PSScriptRoot
$VsCodeSettingsPath = Join-Path $ProjectRoot ".vscode\settings.json"

function Get-VsCodeSettingValue($Name) {
    if (!(Test-Path $VsCodeSettingsPath)) {
        return ""
    }

    $settings = Get-Content -Raw $VsCodeSettingsPath | ConvertFrom-Json
    $property = $settings.PSObject.Properties[$Name]
    if ($null -eq $property) {
        return ""
    }

    return [string]$property.Value
}

function Resolve-ToolchainPath($Path) {
    if (![string]::IsNullOrWhiteSpace($Path)) {
        return $Path
    }

    $Path = Get-VsCodeSettingValue "stm32.gccPath"
    if (![string]::IsNullOrWhiteSpace($Path)) {
        return $Path
    }

    if (![string]::IsNullOrWhiteSpace($env:ARM_GCC_PATH)) {
        return $env:ARM_GCC_PATH
    }

    throw "Missing GCC path. Set stm32.gccPath in .vscode/settings.json or pass -Toolchain."
}

function Resolve-JLinkGdbServerPath($Path) {
    if (![string]::IsNullOrWhiteSpace($Path)) {
        return $Path
    }

    $Path = Get-VsCodeSettingValue "stm32.jlinkGdbServer"
    if (![string]::IsNullOrWhiteSpace($Path)) {
        return $Path
    }

    if (![string]::IsNullOrWhiteSpace($env:JLINK_GDB_SERVER)) {
        return $env:JLINK_GDB_SERVER
    }

    throw "Missing J-Link GDB Server path. Set stm32.jlinkGdbServer in .vscode/settings.json or pass -JLinkGdbServer."
}
