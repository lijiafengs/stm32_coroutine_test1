param(
    [string]$Toolchain = "",
    [string]$Config = ""
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"
$ObjDir = Join-Path $BuildDir "obj"

if ([string]::IsNullOrWhiteSpace($Config)) {
    $Config = Join-Path $PSScriptRoot "build_config.ps1"
}

if (!(Test-Path $Config)) {
    throw "Missing build config: $Config"
}

. $Config

if ($null -eq $BuildConfig) {
    throw "Build config did not define `$BuildConfig: $Config"
}

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

function ConvertTo-RootPath($Path) {
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }
    return Join-Path $Root $Path
}

function Get-ConfigArray($Name) {
    if ($BuildConfig.ContainsKey($Name) -and ($null -ne $BuildConfig[$Name])) {
        return @($BuildConfig[$Name])
    }
    return @()
}

$Defines = Get-ConfigArray "Defines" | ForEach-Object { "-D$_" }
$Includes = Get-ConfigArray "Includes" | ForEach-Object { "-I$(ConvertTo-RootPath $_)" }
$CpuFlags = Get-ConfigArray "CpuFlags"

$CommonFlags = @(
    "-g3",
    "-Og",
    "-ffunction-sections",
    "-fdata-sections",
    "-Wall",
    "-Wextra",
    "-Wno-unused-parameter"
) + $CpuFlags + $Defines + $Includes + (Get-ConfigArray "ExtraCommonFlags")

$CFlags = @("-std=gnu11") + $CommonFlags + (Get-ConfigArray "ExtraCFlags")
$CxxFlags = @(
    "-std=gnu++20",
    "-fno-exceptions",
    "-fno-rtti",
    "-fno-use-cxa-atexit"
) + $CommonFlags + (Get-ConfigArray "ExtraCxxFlags")

$AsmFlags = @(
    "-x", "assembler-with-cpp"
) + $CommonFlags + (Get-ConfigArray "ExtraAsmFlags")

$SourcesC = Get-ConfigArray "SourcesC"
$SourcesCpp = Get-ConfigArray "SourcesCpp"
$SourcesAsm = Get-ConfigArray "SourcesAsm"

function Get-ObjectPath($RelativePath) {
    $safe = $RelativePath -replace '[:\\/\.]', '_'
    return Join-Path $ObjDir "$safe.o"
}

$Objects = @()

foreach ($src in $SourcesC) {
    $obj = Get-ObjectPath $src
    $Objects += $obj
    & $CC @CFlags -c (ConvertTo-RootPath $src) -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

foreach ($src in $SourcesCpp) {
    $obj = Get-ObjectPath $src
    $Objects += $obj
    & $CXX @CxxFlags -c (ConvertTo-RootPath $src) -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

foreach ($src in $SourcesAsm) {
    $obj = Get-ObjectPath $src
    $Objects += $obj
    & $CC @AsmFlags -c (ConvertTo-RootPath $src) -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$FirmwareName = $BuildConfig["FirmwareName"]
if ([string]::IsNullOrWhiteSpace($FirmwareName)) {
    $FirmwareName = "firmware"
}

$Elf = Join-Path $BuildDir "$FirmwareName.elf"
$Hex = Join-Path $BuildDir "$FirmwareName.hex"
$Bin = Join-Path $BuildDir "$FirmwareName.bin"
$Map = Join-Path $BuildDir "$FirmwareName.map"
$LinkerScript = ConvertTo-RootPath $BuildConfig["LinkerScript"]

if (!(Test-Path $LinkerScript)) {
    throw "Missing linker script: $LinkerScript"
}

$LinkFlags = @(
    "-T$LinkerScript",
    "-Wl,-Map=$Map",
    "-Wl,--gc-sections",
    "-Wl,--print-memory-usage",
    "--specs=nano.specs",
    "--specs=nosys.specs"
) + $CpuFlags + (Get-ConfigArray "ExtraLinkFlags")

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
