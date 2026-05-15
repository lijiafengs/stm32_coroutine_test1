# 工程简要说明

本工程是一个面向 `STM32F407ZET6` 的裸机嵌入式示例工程。

## 1. Codex 生成并验证

本工程由 Codex 根据需求生成，包括：

- STM32 裸机工程结构
- C++20 协程指令框架
- HAL/CMSIS 工程内拷贝
- VS Code 编译任务
- J-Link 调试配置
- 中文编译调试说明文档

工程已在当前电脑环境中完成编译验证，并通过 VS Code + J-Link 进入断点调试。

## 2. STM32 上使用 C++20 协程的例子

本工程演示了如何在无操作系统的 STM32 裸机环境中使用 C++20 coroutine。

示例业务模型为：

```text
接收数据 -> 校验协议帧 -> 分发指令 -> 执行动作 -> 非阻塞等待完成标志 -> ACK
```

其中 `MoveStep()` 指令示例展示了：

```text
解析 payload
启动电机动作
co_await 等待电机完成
发送 ACK
```

等待过程中不会阻塞主循环，主循环仍可继续轮询协议、电机状态和协程调度器。

## 3. VS Code + GCC + J-Link 编译调试

本工程使用以下工具链进行编译和调试：

- VS Code
- GNU Arm GCC
- SEGGER J-Link
- J-Link GDB Server
- Eclipse CDT GDB Debug Adapter

迁移到另一台电脑时，工程内已经包含所需 STM32 HAL/CMSIS 源码。通常只需要修改：

```text
.vscode/settings.json
```

中的两个路径：

```json
{
  "stm32.gccPath": "C:\\SysGCC\\arm-eabi\\bin",
  "stm32.jlinkGdbServer": "C:\\Program Files (x86)\\SEGGER\\JLink_V490e\\JLinkGDBServerCL.exe"
}
```

详细编译调试步骤见：

```text
docs/vscode_build_debug_zh.md
```
