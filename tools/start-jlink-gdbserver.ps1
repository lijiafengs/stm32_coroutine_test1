param(
    [string]$JLinkGdbServer = "",
    [string]$Device = "",
    [string]$Interface = "",
    [string]$Speed = "",
    [int]$Port = 0,
    [int]$TimeoutSeconds = 0
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

. (Join-Path $PSScriptRoot "vscode-settings.ps1")
. (Join-Path $PSScriptRoot "jlink_config.ps1")

$JLinkGdbServer = Resolve-JLinkGdbServerPath $JLinkGdbServer
$Device = Resolve-JLinkStringValue $Device "Device"
$Interface = Resolve-JLinkStringValue $Interface "Interface"
$Speed = Resolve-JLinkStringValue $Speed "Speed"
$Port = Resolve-JLinkIntValue $Port "Port"
$TimeoutSeconds = Resolve-JLinkIntValue $TimeoutSeconds "TimeoutSeconds"

if (!(Test-Path $JLinkGdbServer)) {
    throw "J-Link GDB Server not found: $JLinkGdbServer"
}

Get-Process JLinkGDBServerCL -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 300

$OutLog = Join-Path $BuildDir "jlink-vscode.out.log"
$ErrLog = Join-Path $BuildDir "jlink-vscode.err.log"
Remove-Item $OutLog, $ErrLog -Force -ErrorAction SilentlyContinue

$Args = @(
    "-device", $Device,
    "-if", $Interface,
    "-speed", $Speed,
    "-port", "$Port",
    "-singlerun"
)

$Process = Start-Process `
    -FilePath $JLinkGdbServer `
    -ArgumentList $Args `
    -RedirectStandardOutput $OutLog `
    -RedirectStandardError $ErrLog `
    -WindowStyle Hidden `
    -PassThru

$Deadline = (Get-Date).AddSeconds($TimeoutSeconds)
while ((Get-Date) -lt $Deadline) {
    if ($Process.HasExited) {
        $stdout = if (Test-Path $OutLog) { Get-Content -Raw $OutLog } else { "" }
        $stderr = if (Test-Path $ErrLog) { Get-Content -Raw $ErrLog } else { "" }
        throw "J-Link GDB Server exited early with code $($Process.ExitCode).`nSTDOUT:`n$stdout`nSTDERR:`n$stderr"
    }

    $log = if (Test-Path $OutLog) { Get-Content -Raw $OutLog } else { "" }
    if ($log -match "Waiting for GDB connection") {
        Write-Host "J-Link GDB Server is ready on localhost:$Port"
        Write-Host "Log: $OutLog"
        exit 0
    }

    Start-Sleep -Milliseconds 200
}

$tail = if (Test-Path $OutLog) { Get-Content -Raw $OutLog } else { "" }
throw "Timed out waiting for J-Link GDB Server. Log:`n$tail"
