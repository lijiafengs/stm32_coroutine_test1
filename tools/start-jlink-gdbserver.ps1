param(
    [string]$JLinkGdbServer = "C:\Program Files (x86)\SEGGER\JLink_V490e\JLinkGDBServerCL.exe",
    [string]$Device = "STM32F407ZE",
    [string]$Interface = "SWD",
    [string]$Speed = "2000",
    [int]$Port = 2331,
    [int]$TimeoutSeconds = 10
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

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
