# VS Code 编译调试说明

本文档说明如何在 VS Code 中编译和调试本工程。工程已经包含 STM32 HAL、CMSIS、启动文件和链接脚本；迁移到另一台电脑时，只需要配置 GNU Arm GCC 和 J-Link GDB Server 的路径。

## 1. 工程目录

当前工程目录：

```text
C:\Users\...\Desktop\stm32_coroutine_test1
```

主要目录说明：

- `.vscode/`：VS Code 编译、调试、工具路径配置。
- `Drivers/STM32F4xx/`：工程内自带的 STM32 HAL 和 CMSIS 源码。
- `include/`：应用头文件、HAL 配置文件、协程框架接口。
- `src/`：应用源码、协程调度、指令分发、电机模拟。
- `linker/`：STM32F407ZE 链接脚本。
- `tools/`：PowerShell 编译脚本和 J-Link GDB Server 启动脚本。
- `build/`：编译输出目录。

## 2. 新电脑环境要求

新电脑需要安装：

- VS Code。
- GNU Arm Embedded Toolchain，要求包含：
  - `arm-none-eabi-gcc.exe`
  - `arm-none-eabi-g++.exe`
  - `arm-none-eabi-gdb.exe`
  - `arm-none-eabi-objcopy.exe`
  - `arm-none-eabi-size.exe`
- SEGGER J-Link 软件，要求包含：
  - `JLinkGDBServerCL.exe`
- VS Code 扩展：
  - `Eclipse CDT GDB Debug Adapter Extension`

当前调试配置使用的是 Eclipse CDT GDB，也就是 `launch.json` 里的：

```json
"type": "gdbtarget"
```

不要改回 Microsoft `cppdbg`，因为当前环境下 `cppdbg` 曾出现 ARM 架构解析错误。

## 3. 配置工具路径

打开：

```text
.vscode/settings.json
```

根据本机安装位置修改这两个路径：

```json
{
  "stm32.gccPath": "C:\\SysGCC\\arm-eabi\\bin",
  "stm32.jlinkGdbServer": "C:\\Program Files (x86)\\SEGGER\\JLink_V490e\\JLinkGDBServerCL.exe"
}
```

说明：

- `stm32.gccPath` 必须指向 GNU Arm 工具链的 `bin` 目录。
- `stm32.jlinkGdbServer` 必须指向 `JLinkGDBServerCL.exe` 文件本身。

## 4. 手动编译

方式一：快捷键编译。

1. 在 VS Code 打开工程根目录。
2. 按 `Ctrl+Shift+B`。
3. VS Code 会执行默认编译任务 `Build Firmware (GCC)`。

方式二：从 F1 选择编译任务。

1. 按 `F1`。
2. 输入并选择 `Tasks: Run Task`。
3. 选择 `Build Firmware (GCC)`。

方式三：在 VS Code 终端中编译。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1
```

通常不需要在命令行指定 GCC 路径，脚本会读取 `.vscode/settings.json` 中的 `stm32.gccPath`。

如果想临时覆盖 GCC 路径：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1 -Toolchain C:\SysGCC\arm-eabi\bin
```

编译成功后会生成：

```text
build\firmware.elf
build\firmware.hex
build\firmware.bin
build\firmware.map
```

其中：

- `firmware.elf`：调试使用，包含符号信息。
- `firmware.hex`：可用于烧录。
- `firmware.bin`：裸二进制固件。
- `firmware.map`：链接映射文件，用于分析符号和内存占用。

## 5. 手动清理编译输出

1. 按 `F1`。
2. 输入并选择 `Tasks: Run Task`。
3. 选择 `Clean Build Output`。

这个任务会删除 `build/` 目录。

## 6. 只下载固件

如果只想把固件下载到板子上，不想进入 VS Code 调试界面，可以使用只下载任务。

操作步骤：

1. 按 `F1`。
2. 输入并选择 `Tasks: Run Task`。
3. 选择 `Download Firmware (J-Link)`。

这个任务会依次执行：

1. `Build Firmware (GCC)`：先编译生成 `build/firmware.elf`。
2. `J-Link Download Firmware`：启动 J-Link GDB Server，使用 `arm-none-eabi-gdb` 执行 `load` 下载 ELF。
3. 下载完成后执行 reset 和 go，然后退出，不进入断点调试。

也可以在 VS Code 终端中手动执行下载脚本：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\download.ps1
```

注意：

- 只下载任务不会停在 `main`。
- 只下载任务不会打开 VS Code 调试界面。
- J-Link 参数保持和当前调试任务一致：`STM32F407ZE`、`SWD`、`2000 kHz`、端口 `2331`。

## 7. 调试前检查

调试前确认：

- J-Link 已连接电脑。
- J-Link 已连接 STM32F407ZET6 板子的 SWD 接口。
- 目标板已经上电。
- `.vscode/settings.json` 中的两个路径正确。
- VS Code 调试配置选择的是 `J-Link Debug STM32F407ZE (CDT GDB)`。

## 8. 一键调试

1. 打开 VS Code 左侧 `Run and Debug`。
2. 选择调试配置：

```text
J-Link Debug STM32F407ZE (CDT GDB)
```

3. 按 `F5`。

执行过程如下：

1. VS Code 执行 `Debug: Build + Start J-Link GDB Server`。
2. 该任务先执行 `Build Firmware (GCC)` 编译固件。
3. 然后执行 `Start J-Link GDB Server` 启动 J-Link GDB Server。
4. 启动脚本等待日志中出现 `Waiting for GDB connection...`。
5. VS Code 的 CDT GDB 调试器连接 `localhost:2331`。
6. GDB 执行：
   - `monitor reset`
   - `monitor halt`
   - `load`
   - `break main`
   - `monitor reset`
   - `monitor halt`
7. 进入正常断点调试。

## 9. 调试时建议观察的变量

可以在 Watch 窗口添加：

```cpp
app::g_diagnostics
app::g_diagnostics.m_loopCount
app::g_diagnostics.m_framesOk
app::g_diagnostics.m_commandStarted
app::g_diagnostics.m_motorStarted
app::g_diagnostics.m_motorCompleted
app::g_diagnostics.m_ackOk
app::g_diagnostics.m_schedulerResumes
app::g_diagnostics.m_schedulerTimeouts
```

当前 demo 在启动时会注入一帧内存模拟的 `MoveStep` 指令。正常运行后，典型现象是：

- `m_framesOk` 变为 `1`。
- `m_commandStarted` 变为 `1`。
- `motor_started` 变为 `1`。
- `motor_completed` 变为 `1`。
- `ack_ok` 变为 `1`。
- `scheduler_timeouts` 保持 `0`。

## 10. 停止残留的 J-Link GDB Server

如果调试异常退出，可能残留 `JLinkGDBServerCL.exe` 进程。可以手动停止：

1. 按 `F1`。
2. 输入并选择 `Tasks: Run Task`。
3. 选择 `Stop J-Link GDB Server`。

也可以在 PowerShell 中执行：

```powershell
Get-Process JLinkGDBServerCL -ErrorAction SilentlyContinue | Stop-Process -Force
```

## 11. 常见问题

### 10.1 F1 中找不到编译任务

确认 VS Code 打开的是工程根目录，而不是 `src/`、`include/` 等子目录。

正确打开的目录应包含：

```text
.vscode
Drivers
include
src
tools
README.md
```

### 10.2 找不到 GCC

检查 `.vscode/settings.json`：

```json
"stm32.gccPath": "C:\\SysGCC\\arm-eabi\\bin"
```

该目录下必须有：

```text
arm-none-eabi-gcc.exe
arm-none-eabi-g++.exe
arm-none-eabi-gdb.exe
```

### 10.3 找不到 J-Link GDB Server

检查 `.vscode/settings.json`：

```json
"stm32.jlinkGdbServer": "C:\\Program Files (x86)\\SEGGER\\JLink_V490e\\JLinkGDBServerCL.exe"
```

该路径必须指向实际存在的 `JLinkGDBServerCL.exe`。

### 10.4 卡在 Waiting for GDB connection

当前工程使用普通启动脚本，不应该因为 VS Code background task 卡住。如果仍然卡住：

1. 运行 `Stop J-Link GDB Server`。
2. 重新按 `F5`。
3. 查看 `build\jlink-vscode.out.log`。

日志中应有：

```text
Waiting for GDB connection...
```

如果有这行但 VS Code 不进入调试，说明 GDB 调试器没有连接到 `localhost:2331`。

### 10.5 出现 Parameter 'arch' 错误

这是 Microsoft `cppdbg` 调试器在本环境中出现过的问题。本工程当前使用 Eclipse CDT GDB 调试器，配置名称是：

```text
J-Link Debug STM32F407ZE (CDT GDB)
```

不要选择旧的 `cppdbg` 配置。如果 VS Code 下拉列表中仍有旧配置，执行：

```text
F1 -> Developer: Reload Window
```

### 10.6 端口 2331 被占用

先停止残留进程：

```text
F1 -> Tasks: Run Task -> Stop J-Link GDB Server
```

如果仍然不行，可以在任务管理器中结束 `JLinkGDBServerCL.exe`。

## 12. 迁移到另一台电脑的步骤

1. 拷贝整个工程目录。
2. 在新电脑安装 GNU Arm Embedded Toolchain。
3. 在新电脑安装 SEGGER J-Link。
4. 在 VS Code 安装 Eclipse CDT GDB Debug Adapter Extension。
5. 修改 `.vscode/settings.json` 中的：
   - `stm32.gccPath`
   - `stm32.jlinkGdbServer`
6. 打开工程根目录。
7. 执行 `Build Firmware (GCC)`。
8. 选择 `J-Link Debug STM32F407ZE (CDT GDB)` 并按 `F5` 调试。

只要芯片仍是 `STM32F407ZET6`，调试接口仍是 SWD，通常不需要修改其他文件。
