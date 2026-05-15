# 构建配置拆分说明

本工程已将构建脚本拆分为两部分：

- `tools/build.ps1`：通用 GCC 构建流程。
- `tools/build_config.ps1`：工程、芯片、源码和链接脚本配置。

换芯片、换启动文件、增删 HAL 源码、增加业务源码、修改宏定义或 include 路径时，优先只修改：

```text
tools/build_config.ps1
```

正常情况下不需要修改：

```text
tools/build.ps1
```

## 1. 通用构建脚本

`tools/build.ps1` 负责：

- 解析工具链路径。
- 检查 `arm-none-eabi-gcc.exe`、`arm-none-eabi-g++.exe`、`objcopy`、`size`。
- 创建 `build/` 和 `build/obj/`。
- 编译 C 文件。
- 编译 C++ 文件。
- 编译汇编启动文件。
- 链接 ELF。
- 生成 HEX 和 BIN。
- 输出 size 信息。

它不再直接写死：

- 芯片宏。
- CPU/FPU 参数。
- HAL/CMSIS include 路径。
- HAL/CMSIS 源文件。
- 应用源文件。
- startup 文件。
- linker script。

## 2. 工程配置文件

`tools/build_config.ps1` 中的 `$BuildConfig` 保存当前 STM32F407 工程相关配置。

### 2.1 FirmwareName

输出固件文件名：

```powershell
FirmwareName = "firmware"
```

会生成：

```text
build/firmware.elf
build/firmware.hex
build/firmware.bin
build/firmware.map
```

### 2.2 Defines

预处理宏：

```powershell
Defines = @(
    "STM32F407xx",
    "USE_HAL_DRIVER",
    "HSE_VALUE=8000000"
)
```

换芯片时通常需要修改芯片宏，例如：

```powershell
"STM32F407xx"
```

### 2.3 Includes

头文件搜索路径：

```powershell
Includes = @(
    "include",
    "Drivers/STM32F4xx/CMSIS/Core/Include",
    "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Include",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Inc"
)
```

新增模块或换 HAL/CMSIS 目录时，在这里增删路径。

### 2.4 CpuFlags

CPU、FPU 和 ABI 参数：

```powershell
CpuFlags = @(
    "-mcpu=cortex-m4",
    "-mthumb",
    "-mfpu=fpv4-sp-d16",
    "-mfloat-abi=softfp"
)
```

换成其他 Cortex-M 芯片时，优先修改这里。

### 2.5 SourcesC

C 源文件列表：

```powershell
SourcesC = @(
    "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c",
    "Drivers/STM32F4xx/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
)
```

新增 HAL 外设源码时，在这里加入对应 `.c` 文件。

### 2.6 SourcesCpp

C++ 源文件列表：

```powershell
SourcesCpp = @(
    "src/diagnostics.cpp",
    "src/frame.cpp",
    "src/main.cpp"
)
```

新增业务模块 `.cpp` 时，在这里加入文件路径。

### 2.7 SourcesAsm

汇编启动文件列表：

```powershell
SourcesAsm = @(
    "Drivers/STM32F4xx/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f407xx.s"
)
```

换芯片时通常需要换 startup 文件。

### 2.8 LinkerScript

链接脚本：

```powershell
LinkerScript = "linker/STM32F407ZE_FLASH.ld"
```

换芯片、换 Flash/RAM 大小或换内存布局时，修改这里并提供新的 `.ld` 文件。

### 2.9 Extra Flags

额外编译或链接参数：

```powershell
ExtraCommonFlags = @()
ExtraCFlags = @()
ExtraCxxFlags = @()
ExtraAsmFlags = @()
ExtraLinkFlags = @()
```

仅当某个芯片、库或模块需要额外选项时使用。

## 3. 手动指定配置文件

默认构建会自动读取：

```text
tools/build_config.ps1
```

也可以手动指定另一个配置文件：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1 -Config .\tools\build_config_stm32f407.ps1
```

这适合保留多个芯片配置文件，例如：

```text
tools/build_config_stm32f407.ps1
tools/build_config_stm32f103.ps1
tools/build_config_stm32h743.ps1
```

## 4. VS Code 构建

VS Code 的 `Build Firmware (GCC)` 任务仍然调用：

```text
tools/build.ps1
```

因此 VS Code 使用的默认配置也是：

```text
tools/build_config.ps1
```

如果要让 VS Code 使用其他配置文件，可以在 `.vscode/tasks.json` 中给 `build.ps1` 增加 `-Config` 参数。

