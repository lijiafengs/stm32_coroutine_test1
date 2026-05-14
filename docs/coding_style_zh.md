# C++ 嵌入式工程代码风格说明

本文档总结本工程当前采用的代码风格要求，后续新增代码、重构代码和生成代码时应遵守这些规则。

## 1. 命名风格

### 1.1 变量命名

成员变量使用 `m_` 前缀：

```cpp
std::int32_t m_currentPos = 0;
bool m_isBusy = false;
```

全局变量使用 `g_` 前缀：

```cpp
std::uint32_t g_currentCount = 0;
Diagnostics g_diagnostics{};
```

局部变量不强制加前缀，使用清晰的驼峰命名：

```cpp
const std::uint8_t motorId = frame.m_payload[0];
```

### 1.2 函数命名

函数使用 PascalCase：

```cpp
void MovePos();
bool Push(std::uint8_t byte);
void PollMotors(std::uint32_t now);
```

类成员函数、普通函数、静态辅助函数都遵守该规则。

例外：

- `main`
- STM32 HAL/中断要求的回调或入口，如 `SysTick_Handler`、`HAL_MspInit`
- C++ 协程标准要求的 `promise_type` 固定函数名，如 `get_return_object()`、`initial_suspend()`、`final_suspend()`、`return_void()`、`unhandled_exception()`
- 运算符重载函数，如 `operator new`、`operator delete`、`operator=`
- 协程 awaiter 标准接口，如 `await_ready()`、`await_suspend()`、`await_resume()`

### 1.3 枚举命名

枚举类型名不加额外前缀，保持普通类型命名：

```cpp
enum class Status : std::uint8_t
{
    eOk = 0x00,
    eBusy = 0x03,
};
```

枚举值必须加 `e` 前缀：

```cpp
Command::eMoveStep
Status::eOk
Status::eTimeout
```

## 2. 括号风格

所有花括号单独成行，采用 Allman 风格：

```cpp
if (m_isBusy)
{
    return false;
}
```

函数、类、结构体、枚举、命名空间、循环、条件分支、`switch` 都使用同样规则：

```cpp
namespace app
{

    class Motor
    {
    public:
        bool Move(std::int32_t steps);
    };

}
```

不要写成：

```cpp
if (m_isBusy) {
    return false;
}
```

右花括号也应单独成行，不在同一行追加 namespace 尾注释：

```cpp
}
```

不要写成：

```cpp
} // namespace app
```

## 3. 空格和缩进

缩进统一使用 4 个 space。

禁止使用 tab。

每一层嵌套增加 4 个空格。命名空间内部也要缩进：

```cpp
namespace app
{

    namespace
    {

        bool TimeReached(std::uint32_t now, std::uint32_t deadline)
        {
            return static_cast<std::int32_t>(now - deadline) >= 0;
        }

    }

}
```

嵌套类、嵌套结构体、私有枚举同样按层级缩进：

```cpp
class FrameParser
{
public:
    bool Push(std::uint8_t byte);

private:
    enum class State : std::uint8_t
    {
        eStart,
        ePayload,
        eCrc
    };
};
```

## 4. Doxygen 注释风格

### 4.1 文件顶部注释

每个头文件和源文件最上方都需要 Doxygen 文件注释。

文件顶部注释包含：

- `@brief`
- `@author`
- `@date`
- `@version`

其中：

- `author` 固定写 `codex`
- `date` 写本次修改日期
- 不写 `@file`

示例：

```cpp
/**
* @brief Declares command handlers and opcode dispatcher.
* @author codex
* @date 2026-05-14
* @version 1.0
*/
```

### 4.2 函数注释

每个头文件和源文件中的函数都需要 Doxygen 注释。

函数注释包含：

- `@brief`
- `@param`
- `@return`

`@param` 必须标明方向：

- `@param[in]`
- `@param[out]`
- `@param[in,out]`

示例：

```cpp
/**
* @brief Pushes one received byte into the parser state machine.
* @param[in] byte Received byte to process.
* @return True when the byte was accepted, otherwise false on parse error.
*/
bool Push(std::uint8_t byte);
```

无参数函数也要写参数说明：

```cpp
/**
* @brief Polls waiting and ready coroutine queues once.
* @param[in] None No input parameters.
* @return None.
*/
void Poll();
```

无返回值函数写：

```cpp
* @return None.
```

### 4.3 注释缩进

Doxygen 注释也遵守 4 空格缩进。

为了满足严格的 4 空格规则，注释续行不额外保留一个空格：

```cpp
/**
* @brief File level brief.
* @author codex
* @date 2026-05-14
* @version 1.0
*/
```

嵌套作用域中的函数注释按代码层级缩进：

```cpp
    /**
    * @brief Starts a simulated motor move.
    * @param[in] steps Number of relative steps to record.
    * @return True when the move starts, otherwise false.
    */
    bool Move(std::int32_t steps);
```

## 5. 第三方源码处理

不要批量修改第三方源码目录：

```text
Drivers/STM32F4xx
```

STM32 HAL/CMSIS 源码保持供应商原始风格。

本工程自有代码目录可以按本文档风格调整：

```text
include/
src/
```

本地 HAL 配置文件属于工程配置文件，可以按项目风格维护：

```text
include/stm32f4xx_hal_conf.h
```

## 6. 格式化工具

本工程使用 `.clang-format` 固化主要格式规则：

- 4 空格缩进
- 禁止 tab
- 命名空间内部缩进
- Allman 花括号
- 不自动追加 namespace 尾注释

格式化命令示例：

```powershell
& 'C:\Program Files\LLVM\bin\clang-format.exe' -i include\*.hpp src\*.cpp
```

注意：格式化后仍需人工确认命名规则、Doxygen 内容和注释日期。

