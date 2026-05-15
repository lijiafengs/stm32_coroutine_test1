# STM32 应用下载起点 0x0800C000 配置记录

本文记录当应用程序下载/运行起点设置为 `0x0800C000` 时，需要同步调整的工程配置。

## 1. GCC 链接脚本

如果工程使用 GCC 链接脚本，例如 `linker/STM32F407ZE_FLASH.ld`，需要把 Flash 起始地址从 `0x08000000` 改为 `0x0800C000`。

原配置示例：

```ld
FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
```

调整后：

```ld
FLASH (rx) : ORIGIN = 0x0800C000, LENGTH = 464K
```

计算依据：

```text
0x0800C000 - 0x08000000 = 0xC000 = 48K
512K - 48K = 464K = 0x74000
```

如果实际芯片 Flash 容量不是 512K，需要按实际容量重新计算 `LENGTH`。

## 2. 向量表偏移

应用程序启动后，必须让中断向量表指向新的应用起始地址。

最终效果必须等价于：

```c
SCB->VTOR = 0x0800C000;
```

如果使用 STM32 官方 `system_stm32xxxx.c`，通常配置为：

```c
#define USER_VECT_TAB_ADDRESS
#define VECT_TAB_OFFSET  0x0000C000U
```

对应关系：

```c
SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
```

其中 `FLASH_BASE` 为 `0x08000000`，`VECT_TAB_OFFSET` 为 `0x0000C000`。

注意：`VECT_TAB_OFFSET` 需要满足芯片对向量表地址对齐的要求。`0xC000` 满足 STM32F4 常见对齐要求。

## 3. 下载和调试配置

如果下载的是 `.elf` 或 `.hex` 文件，只要链接脚本已经改为 `0x0800C000`，下载地址会自动跟随 ELF/HEX 中的地址信息。

如果下载的是裸 `.bin` 文件，必须显式指定下载地址：

```text
loadbin firmware.bin 0x0800C000
```

或在 ST-LINK/J-Link 下载工具中把下载地址填写为：

```text
0x0800C000
```

## 4. Keil 配置参考

如果工程使用 Keil，需要在 Target 配置中调整 IROM1：

```text
IROM1 Start: 0x0800C000
IROM1 Size : 总Flash大小 - 0xC000
```

以 512K Flash 为例：

```text
IROM1 Start: 0x0800C000
IROM1 Size : 0x74000
```

## 5. Bootloader 跳转 App

如果 `0x08000000` 到 `0x0800BFFF` 区域由 Bootloader 占用，Bootloader 跳转应用时应使用应用起始地址 `0x0800C000`。

典型跳转代码：

```c
#define APP_ADDR 0x0800C000U

typedef void (*app_entry_t)(void);

void jump_to_app(void)
{
    uint32_t app_stack = *(uint32_t *)APP_ADDR;
    uint32_t app_reset = *(uint32_t *)(APP_ADDR + 4U);

    __disable_irq();
    __set_MSP(app_stack);
    ((app_entry_t)app_reset)();
}
```

实际工程中还应在跳转前关闭外设、中断、SysTick，并根据需要复位时钟配置。

## 6. 必须保持一致的地址

以下三处必须统一为 `0x0800C000` 相关配置：

- 链接地址：应用程序链接到 `0x0800C000`
- 向量表地址：`SCB->VTOR = 0x0800C000`
- 下载地址：裸 `.bin` 下载时指定 `0x0800C000`

如果三者不一致，常见现象包括：

- 下载成功但程序无法正常运行
- 进入中断后跑飞
- Bootloader 跳转后 HardFault
- 复位后仍停留在 Bootloader 或进入异常入口
