# STM32F407ZET6 C++20 协程指令框架计划

## Summary
- 在 `C:\Users\...\Desktop\stm32_test3` 新建 VS Code 裸机工程，目标 `STM32F407ZET6 / STM32F407ZE`。
- 编译和 GDB 使用 `C:\SysGCC\arm-eabi\bin`。
- HAL/CMSIS 主库使用 `C:\Users\...\Desktop\...\Drivers\STM32F4xx`，因为这里已确认有完整 `HAL`、`CMSIS`、`stm32f407xx.h`、GCC startup。
- VisualGDB BSP 作为辅助来源：`C:\Users\...\AppData\Local\VisualGDB\EmbeddedBSPs\arm-eabi\com.sysprogs.arm.stm32\STM32F4xxxx`，优先参考其中 `STM32F407ZE_flash.lds` 和 `startup_stm32f407xx.c`；如果主库编译不顺，再切换或补齐。
- 调试使用 `JLinkGDBServerCL.exe` + `arm-none-eabi-gdb.exe`，VS Code 一键下载和断点调试。

## Key Changes
- 工程文件：
  - `.vscode/tasks.json`：调用 `tools/build.ps1` 编译。
  - `.vscode/launch.json`：J-Link 连接 `STM32F407ZE`，下载 ELF，停在 `main`。
  - `linker/STM32F407ZE_FLASH.ld`：从 VisualGDB 的 `STM32F407ZE_flash.lds` 改成工程本地链接脚本。
  - `src/`、`include/`：启动、HAL 初始化、主循环、协程调度、协议帧、指令分发、电机接口。
- HAL 接入：
  - Include 使用 本地工程"..." 里的 `CMSIS\Core\Include`、`CMSIS\Device\ST\STM32F4xx\Include`、`STM32F4xx_HAL_Driver\Inc`。
  - 编译最小 HAL 源：`stm32f4xx_hal.c`、`stm32f4xx_hal_cortex.c`、`stm32f4xx_hal_rcc.c`、`stm32f4xx_hal_gpio.c`。
  - 使用 `STM32F407xx` 宏和本地 `stm32f4xx_hal_conf.h`。
- 协程框架：
  - `Task<void>` 基于 C++20 coroutine handle。
  - 固定任务槽池，不用 heap、异常、RTTI。
  - `Scheduler::Poll()` 在主循环恢复 ready 协程。
  - `co_await WaitFlag` / `co_await MotorDone` 实现非阻塞等待。
- 指令和电机示例：
  - `CommandDispatcher` 按 opcode 调用 `Commands::MoveStep()`。
  - `MoveStep()`：解析 payload -> `GetMotor(1).Move(100)` -> 等待完成 -> ACK。
  - `SimMotor` 用 tick 模拟 busy/done；后续真实电机只替换 `IMotor` 实现。

## Test Plan
- 编译：运行 `tools/build.ps1` 或 VS Code build task，生成 `build/firmware.elf/.hex/.bin`。
- 若 MotorCtrl 这份 HAL 缺文件或版本不兼容，先用 VisualGDB BSP 中对应 HAL/linker/startup 补齐，并记录具体原因。
- 板上调试：
  - VS Code 启动 J-Link 调试配置。
  - 下载到 STM32F407ZET6。
  - 断点检查 `main`、`CommandDispatcher::Dispatch`、`Commands::MoveStep`、`Scheduler::Poll`。
  - 运行后确认 demo 指令启动、电机等待期间主循环不阻塞、完成后 ACK 写入。
- 错误场景：CRC 错、未知 opcode、任务槽池满、电机超时均返回明确状态。

## Assumptions
- 不额外下载 HAL/CMSIS。
- 本阶段不配置 UART，也不输出真实 STEP/DIR 脉冲。
- VisualGDB BSP 可作为备选和参考，但主工程不依赖 VisualGDB IDE。
