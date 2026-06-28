# Yorulog

一个面向 STM32 HAL 的 **极简、单头文件** UART 日志库，目标是在 **尽可能低的 Flash / RAM 占用** 下，提供 **日志分级、前缀、可选时间戳**，以及可选的 **FULL 模式**（环形缓冲 + 自动 DMA 发送）。

> 不使用 `printf`  
> 不引入格式化解析器  
> 不依赖沉重的标准库  
>  
> 只要一个头文件就能开始打日志。

---

## 特性

- **单头文件**：`yorulog.h`
- **仅依赖 HAL**：基于 UART，适用于多种 STM32 系列
- **两种模式**
  - **MINI**：极小体积，阻塞直发
  - **FULL**：环形缓冲；当 `huart->hdmatx` 存在时自动启用 DMA TX
- **日志等级**：`ERROR / WARN / INFO / DEBUG / TRACE`
- **分级前缀**：`[E] [W] [I] [D] [T]`
- **可选时间戳**：`[HAL_GetTick()]`
- **基于 C11 `_Generic` 的类型自适应** `YORULOG_Print()` / `YORULOG_Println()`
- **平台感知能力**
  - 自动探测 STM32 HAL 头文件
  - 在 STM32H7 上默认自动启用 DMA 可访问缓冲区支持

---

## 为什么不用 `printf`？

在小型 MCU 上，`printf` 往往会引入体积不小的格式化运行时，同时带来更高的栈占用和更不可预测的代码尺寸。

Yorulog 完全绕开格式化解析，因此代码大小和运行行为都更简单、更可控。

---

## 快速开始

### 1. 添加 `yorulog.h`

把头文件放到工程里，例如：

```text
/Core/easy-ST-Logger/yorulog.h
```

### 2. 在包含前定义配置宏

必须在且仅在 **一个** `.c` 文件里，在包含 `yorulog.h` 前定义 `YORULOG_DEFINE_GLOBALS`：

```c
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

其他源文件里正常包含即可：

```c
#include "yorulog.h"
```

### 3. 选择模式

#### MINI 模式

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

#### FULL 模式

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

### 4. 使用 UART 句柄初始化

```c
extern UART_HandleTypeDef huart1;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();

    YORULOG_Init(&huart1);
    YORULOG_SetLevel(YORULOG_LEVEL_DEBUG);

    YORULOG_LogInfo("boot");
    YORULOG_LogDebug(123);
    YORULOG_LogWarn((void*)0x20000000);

    while (1) { }
}
```

---

## FULL 模式与 DMA

如果 `huart->hdmatx != NULL`，FULL 模式会自动使用 DMA TX。

你需要在 HAL 的发送完成回调中转发：

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}
```

如果没有配置 TX DMA，FULL 模式会自动回退到阻塞发送。

---

## STM32H7 说明

在 STM32H7 上，很多工程会把默认的 `.data/.bss` 放在 DTCM 中，而 DMA 无法访问 DTCM。  
因此 Yorulog 会在 H7 上默认自动启用 **DMA 可访问发送缓冲区**。

默认的 H7 缓冲区 section 为：

```c
#define YORULOG_DMA_SECTION ".RAM_D2"
```

你的链接脚本需要把这个 section 放到 DMA 可访问的 RAM，例如：

```ld
.RAM_D2 (NOLOAD) :
{
  . = ALIGN(32);
  KEEP(*(.RAM_D2))
  KEEP(*(.RAM_D2*))
  . = ALIGN(32);
} >RAM_D2
```

如果 H7 工程启用了 D-Cache，DMA 缓冲区一致性仍可能需要 MPU non-cacheable 配置或显式 cache 维护。

---

## API

### 日志宏

```c
YORULOG_LogError("fatal");
YORULOG_LogWarn("warn");
YORULOG_LogInfo("info");
YORULOG_LogDebug("debug");
YORULOG_LogTrace("trace");
```

每个宏接收一个自动识别类型的参数：

- `const char*` / `char*`
- `char`
- 有符号 / 无符号整数
- 指针

### 原始输出

```c
YORULOG_Print("x=");
YORULOG_Print(123);
YORULOG_Println("done");
```

### 运行时设置日志等级

```c
YORULOG_SetLevel(YORULOG_LEVEL_INFO);
```

### 手动刷新

```c
YORULOG_Flush();
```

---

## 配置宏

在 `#include "yorulog.h"` 之前定义。

| 宏 | 默认值 | 说明 |
|---|---:|---|
| `YORULOG_ENABLE` | `1` | 总开关 |
| `YORULOG_MINI` | `0` | `1` = MINI，`0` = FULL |
| `YORULOG_TX_BUF_SIZE` | `512` | FULL 模式环形缓冲区大小 |
| `YORULOG_DEFAULT_LEVEL` | `4` | 启动日志等级，即 `TRACE` |
| `YORULOG_TIMESTAMP` | `0` | 前缀时间戳 `[ms]` |
| `YORULOG_NOW_MS()` | `HAL_GetTick()` | 自定义时间源 |
| `YORULOG_CRLF` | `1` | 使用 `\r\n` |
| `YORULOG_DROP_NEW_ON_FULL` | `0` | FULL 模式下，在非阻塞策略里覆盖旧数据而不是丢新数据 |
| `YORULOG_BLOCK_ON_FULL` | `1` | FULL 模式下，缓冲区满时阻塞/刷新 |
| `YORULOG_FORCE_BLOCKING_EW` | `1` | `ERROR/WARN` 强制刷新/阻塞 |
| `YORULOG_DMA_SECTION` | H7 上默认为 `".RAM_D2"` | DMA 缓冲区 section 名 |
| `YORULOG_PREFIX_E/W/I/D/T` | `"[E] "` 等 | 各级别前缀 |

当前版本仍保留兼容层，旧的 `STLOG_*` 宏以及 `stlog_*` / `log*` API 仍可继续使用。

---

## 行为说明

### MINI 模式

- 始终使用阻塞式 UART 发送
- 体积最小，逻辑最简单
- 适合资源非常紧张的量产固件

### FULL 模式

- 使用环形缓冲
- 存在 DMA TX 时会自动切换到 DMA 发送
- 默认策略优先保证 **日志完整性**，而不是追求峰值下完全不阻塞
- 按当前默认配置，缓冲区满时会阻塞/刷新，而不是静默截断输出

---

## 实测体积开销

测试平台：**STM32H743**，使用本仓库当前测试工程的 **Release** 预设。  
编译条件：

- `-Os -g0`
- `--specs=nano.specs`
- `-ffunction-sections -fdata-sections -Wl,--gc-sections`

### 基线（仅 HAL + USART1）

| 资源 | 占用 |
|---|---:|
| Flash | 15536 B |
| DTCMRAM | 1856 B |
| RAM_D2 | 0 B |
| `arm-none-eabi-size` dec | 17376 |

### MINI 模式（仅接入库，调用 `YORULOG_Init()`）

| 资源 | 占用 | 增量 |
|---|---:|---:|
| Flash | 15560 B | +24 B |
| DTCMRAM | 1864 B | +8 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 17920 | +544 |

### FULL 模式（接入库，接好 DMA 回调，不运行自测输出）

| 资源 | 占用 | 增量 |
|---|---:|---:|
| Flash | 17092 B | +1556 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 19476 | +2100 |

### FULL 模式 + `main.c` 自带自测

| 资源 | 占用 | 相对基线增量 |
|---|---:|---:|
| Flash | 18532 B | +2996 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 20916 | +3540 |

说明：

- 自测字符串和演示逻辑本身，额外增加了大约 `1440 B` Flash。
- 按当前实现，`YORULOG_DEFINE_GLOBALS` 仍会保留全局 `yorulog_tx_buf`，所以 `MINI` 模式下也会看到 `512 B` 的 `RAM_D2` 占用。
