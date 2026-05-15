# STM32F407 协程指令框架对话记录与最终计划

本文档整理本工程从需求提出、方案确认、实现、调试到 GitHub 上传准备阶段的主要沟通记录和最终方案。记录时间为 2026-05-14，工程目录已由 `stm32_test3` 重命名为 `stm32_coroutine_test1`。

## 1. 原始需求

用户希望设计一个用于无操作系统嵌入式系统的 C++20 协程框架，目标板为 `STM32F407ZET6`，通过 J-Link 连接到电脑。

核心业务模型如下：

- 系统没有操作系统，主循环持续运行。
- 主循环不断读取接收数据。
- 当收到一帧数据并校验通过后，解析数据中的指令值。
- 根据指令值调用对应的指令函数。
- 指令函数通常是：
  ```text
  do something -> wait done flag -> ack
  ```
- `do something` 之后不能阻塞主循环。
- 系统还有电机执行类。
- 示例指令 `MoveStep()` 的逻辑为：
  ```text
  parse data();
  GetMotor(1).Move(100);
  wait motor complete flag;
  ack;
  ```
- 等待电机完成期间，主循环必须继续运行。

## 2. 工具链与库路径确认

用户提供的编译调试工具链路径：

```text
<GNU Arm GCC 安装路径>
```

用户提供和确认可用的 STM32 HAL/CMSIS 来源：

```text
C:\Users\...\STM32F4xx
C:\Users\...\AppData\Local\VisualGDB\EmbeddedBSPs
```

最终决定：

- 主 HAL/CMSIS 来源使用 `...\Drivers\STM32F4xx`。
- VisualGDB BSP 作为链接脚本和启动文件参考来源。
- 不额外下载 STM32 HAL/CMSIS。
- 工程必须自包含，使用到的 STM32 库源码全部复制进工程，不能只引用外部路径。
- 工程迁移到另一台电脑后，只需要配置 GCC 和 J-Link 路径即可编译调试。

## 3. 最终工程位置

初始计划中新建目录为：

```text
C:\Users\...\Desktop\stm32_test3
```

之后用户将工程重命名为：

```text
C:\Users\...\Desktop\stm32_coroutine_test1
```

后续文档、Git 仓库和上传目标均以 `stm32_coroutine_test1` 为准。

## 4. 最终实施计划

### 4.1 工程目标

建立一个 VS Code 裸机 STM32F407ZET6 C++20 固件工程，使用 GNU Arm GCC 编译，使用 J-Link GDB Server 和 VS Code 调试，提供一个可迁移、可编译、可断点调试的协程指令框架示例。

### 4.2 工程文件

工程包含以下主要目录和文件：

- `.vscode/tasks.json`：VS Code 编译、清理、启动和停止 J-Link GDB Server 任务。
- `.vscode/launch.json`：VS Code 调试配置，连接 J-Link GDB Server 并下载 ELF。
- `.vscode/settings.json`：保存本机 GCC 和 J-Link 路径，迁移时主要修改这里。
- `tools/build.ps1`：PowerShell 编译脚本。
- `tools/start_jlink_gdb_server.ps1`：启动 J-Link GDB Server 并等待 GDB 连接端口就绪。
- `linker/STM32F407ZE_FLASH.ld`：本地链接脚本。
- `Drivers/STM32F4xx/`：工程内自带 HAL/CMSIS 源码。
- `include/`：应用头文件、HAL 配置、协程框架接口。
- `src/`：启动、HAL 初始化、主循环、协议、调度器、指令、电机示例实现。
- `docs/vscode_build_debug_zh.md`：中文编译调试说明。

### 4.3 HAL/CMSIS 接入

工程内复制并使用以下 STM32 库内容：

- `CMSIS\Core\Include`
- `CMSIS\Device\ST\STM32F4xx\Include`
- `STM32F4xx_HAL_Driver\Inc`
- `STM32F4xx_HAL_Driver\Src`
- GCC 启动文件和链接脚本所需内容

编译时使用：

```text
STM32F407xx
```

并使用本地 `stm32f4xx_hal_conf.h`。

最小 HAL 源码集合包括：

- `stm32f4xx_hal.c`
- `stm32f4xx_hal_cortex.c`
- `stm32f4xx_hal_rcc.c`
- `stm32f4xx_hal_gpio.c`

### 4.4 协程框架设计

协程框架采用 C++20 coroutine，满足裸机无 OS 使用场景：

- `Task<void>` 基于 `std::coroutine_handle`。
- 不依赖 heap。
- 不启用异常。
- 不启用 RTTI。
- 使用固定任务槽池管理运行中的协程任务。
- `Scheduler::Poll()` 在主循环中被反复调用。
- `Scheduler::Poll()` 只恢复已经 ready 的协程，不阻塞主循环。
- 通过 awaiter 实现非阻塞等待。

典型 awaiter：

```text
co_await WaitFlag(...)
co_await MotorDone(...)
```

设计目标是让指令函数可以写成接近同步流程的形式，但运行时仍然是非阻塞状态机。

### 4.5 指令分发和电机示例

指令框架包含：

- 协议帧解析。
- CRC 或校验错误处理。
- opcode 指令分发。
- 未知 opcode 处理。
- ACK/NACK 返回。
- 任务槽满处理。
- 电机完成等待和超时处理。

示例指令：

```text
Commands::MoveStep()
```

示例逻辑：

```text
解析 payload
GetMotor(1).Move(100)
co_await 等待电机完成
ACK
```

当前阶段没有配置真实 UART，也没有输出真实 STEP/DIR 脉冲。电机使用 `SimMotor` 通过 tick 模拟 busy/done。后续接真实电机时，只需要替换 `IMotor` 的具体实现。

### 4.6 主循环模型

主循环逻辑为：

```text
HAL_Init 和系统初始化
初始化应用模块
注入 demo 指令或等待真实接收数据
while (true) {
    更新 tick
    读取或处理接收数据
    协议帧校验和解析
    分发指令
    更新电机状态
    Scheduler::Poll()
}
```

电机等待期间，主循环仍持续执行，因此后续可以继续接收数据、处理其他任务、刷新状态。

## 5. VS Code 编译与调试决策记录

### 5.1 编译入口

用户要求能在 VS Code 中点击或通过 F1 选择 build。

最终实现：

- `Ctrl+Shift+B` 可执行默认编译任务。
- `F1` -> `Tasks: Run Task` -> `Build Firmware (GCC)` 可手动选择编译。
- 终端可直接运行：
  ```powershell
  powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1
  ```

### 5.2 调试配置问题和处理

调试过程中出现过以下问题：

1. `unable to start debugging. Debug server process failed to initialize`
2. `unknown command line parameter <错误的 JLinkGDBServerCL.exe 参数> found`
3. J-Link GDB Server 一直停在：
   ```text
   Waiting for GDB connection...
   ```
4. VS Code 调试报错：
   ```text
   Specified argument was out of the range of valid values. (Parameter 'arch')
   ```

最终处理结果：

- J-Link GDB Server 不再直接由错误的调试器参数启动。
- 改为由 VS Code task 启动 J-Link GDB Server。
- task 等待 GDB Server 就绪后，调试器再连接 `localhost:2331`。
- 调试配置使用 Eclipse CDT GDB Debug Adapter 的 `gdbtarget`。
- 不再使用 Microsoft `cppdbg`，因为当前环境下它出现 ARM 架构参数解析错误。

最终用户确认：

```text
好了，终于可以正常打断点调试了。
```

## 6. 迁移到另一台电脑的结论

用户再次确认迁移问题。

结论：

工程已经自带 STM32 HAL/CMSIS、启动文件、链接脚本、源代码和 VS Code 配置模板。迁移到另一台电脑后，原则上只需要：

- 安装 GNU Arm Embedded Toolchain。
- 安装 SEGGER J-Link 软件。
- 安装 VS Code。
- 安装 Eclipse CDT GDB Debug Adapter Extension。
- 修改 `.vscode/settings.json` 中的两个路径：
  ```json
  {
    "stm32.gccPath": "<GNU Arm GCC 的 bin 目录>",
    "stm32.jlinkGdbServer": "<JLinkGDBServerCL.exe 的完整路径>"
  }
  ```

除这两个工具路径外，不应依赖原电脑上的 HAL/CMSIS 外部路径。

## 7. 已生成的中文说明文档

已按用户要求创建中文编译调试说明：

```text
C:\Users\...\Desktop\stm32_coroutine_test1\docs\vscode_build_debug_zh.md
```

该文档说明了：

- 新电脑环境要求。
- 如何配置 GCC 和 J-Link 路径。
- 如何手动编译。
- 如何手动清理。
- 如何启动调试。
- 常见调试问题。
- 当前 demo 的运行观察点。


## 10. 最终交付标准

本工程的交付目标为：

- 可以在本机 VS Code 中编译。
- 可以使用 J-Link 下载和断点调试。
- 工程源码自包含，HAL/CMSIS 不依赖外部目录。
- 迁移到另一台电脑后，只需调整 GCC 和 J-Link 路径。
- 协程指令框架展示了非阻塞 `do something -> wait flag -> ack` 流程。
- `MoveStep()` 示例展示了电机动作、等待完成和 ACK 的完整链路。
- 中文说明文档可指导用户手动编译和调试。
