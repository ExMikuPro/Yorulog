# Yorulog

Yorulog 是一个面向 STM32 HAL 的轻量级 UART 日志库。

它只有一个头文件，不依赖 `printf`，也不引入额外的格式化解析器。设计目标是尽量降低 Flash、RAM 和栈占用，同时提供嵌入式开发中常用的日志能力，例如日志等级、等级前缀、可选时间戳，以及基于 DMA 的异步发送。

适合用在资源比较紧张、但又需要保留基本调试输出能力的 STM32 项目中。

---

## 主要特性

* 单头文件：只需要引入 `yorulog.h`
* 基于 STM32 HAL UART
* 不使用 `printf`
* 不依赖复杂的标准库格式化能力
* 支持两种工作模式：

  * MINI：阻塞直发，体积最小
  * FULL：环形缓冲，可自动使用 UART TX DMA
* 支持日志等级：

  * `ERROR`
  * `WARN`
  * `INFO`
  * `DEBUG`
  * `TRACE`
* 支持等级前缀：

  * `[E]`
  * `[W]`
  * `[I]`
  * `[D]`
  * `[T]`
* 支持可选时间戳
* 支持 `YORULOG_Print()` / `YORULOG_Println()` 自动识别常见类型
* 可自动探测 STM32 HAL 头文件
* 在 STM32H7 上默认支持 DMA 可访问缓冲区

---

## 为什么不使用 `printf`

在小型 MCU 上，`printf` 很方便，但代价也比较明显。

它通常会额外引入格式化运行时，带来更大的 Flash 占用、更高的栈占用，以及不太容易预测的代码体积。对于只想输出简单日志的固件来说，这些开销有时并不值得。

Yorulog 的思路是绕开格式化解析，只提供常用类型的直接输出能力。这样实现更简单，行为也更可控。

---

## 快速开始

### 1. 添加头文件

把 `yorulog.h` 放到你的工程目录中，例如：

```text
/Core/Yorulog/yorulog.h
```

### 2. 在一个 `.c` 文件中定义全局实现

必须且只需要在一个 `.c` 文件中，在包含头文件前定义：

```c
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

其他 `.c` 文件正常包含即可：

```c
#include "yorulog.h"
```

---

## 选择工作模式

### MINI 模式

MINI 模式使用阻塞式 UART 发送。它的逻辑最简单，占用也最低，适合资源非常紧张或只需要少量日志的项目。

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

### FULL 模式

FULL 模式使用环形缓冲区。如果 UART 句柄中配置了 TX DMA，Yorulog 会自动使用 DMA 发送；如果没有配置 DMA，则会回退到阻塞发送。

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

---

## 基本用法

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

    while (1)
    {
    }
}
```

---

## FULL 模式与 DMA

在 FULL 模式下，如果 `huart->hdmatx != NULL`，Yorulog 会自动使用 DMA TX。

需要注意的是，HAL 的发送完成回调需要转发给 Yorulog：

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}
```

如果没有配置 TX DMA，FULL 模式仍然可以工作，只是会自动回退到阻塞发送。

---

## STM32H7 注意事项

STM32H7 的内存结构比较特殊。很多工程会把默认的 `.data` / `.bss` 放在 DTCM 中，但 DMA 不能访问 DTCM。

因此，Yorulog 在 STM32H7 上默认会把 DMA 发送缓冲区放到 DMA 可访问的 section 中。

默认 section 名称为：

```c
#define YORULOG_DMA_SECTION ".RAM_D2"
```

你的链接脚本需要把这个 section 放到 DMA 可访问的 RAM 中，例如：

```ld
.RAM_D2 (NOLOAD) :
{
  . = ALIGN(32);
  KEEP(*(.RAM_D2))
  KEEP(*(.RAM_D2*))
  . = ALIGN(32);
} >RAM_D2
```

如果工程启用了 D-Cache，还需要注意 DMA 缓冲区一致性问题。可以使用 MPU 把对应区域配置为 non-cacheable，或者在发送前后手动维护 cache。

---

## API

### 日志输出

```c
YORULOG_LogError("fatal");
YORULOG_LogWarn("warn");
YORULOG_LogInfo("info");
YORULOG_LogDebug("debug");
YORULOG_LogTrace("trace");
```

日志宏接收一个参数，并会自动识别常见类型：

* `const char*`
* `char*`
* `char`
* 有符号整数
* 无符号整数
* 指针

---

### 原始输出

```c
YORULOG_Print("x=");
YORULOG_Print(123);
YORULOG_Println("done");
```

`YORULOG_Print()` 不会自动换行。
`YORULOG_Println()` 会在输出后追加换行。

---

### 设置日志等级

```c
YORULOG_SetLevel(YORULOG_LEVEL_INFO);
```

只有等级不高于当前设置的日志会被输出。

例如设置为 `YORULOG_LEVEL_INFO` 后，`ERROR`、`WARN`、`INFO` 会输出，而 `DEBUG` 和 `TRACE` 会被忽略。

---

### 手动刷新

```c
YORULOG_Flush();
```

在 FULL 模式下，可以使用它主动刷新缓冲区内容。

---

## 配置宏

所有配置宏都需要在包含 `yorulog.h` 之前定义。

| 宏                           |                默认值 | 说明                          |
| --------------------------- | -----------------: | --------------------------- |
| `YORULOG_ENABLE`            |                `1` | 总开关                         |
| `YORULOG_MINI`              |                `0` | `1` 为 MINI 模式，`0` 为 FULL 模式 |
| `YORULOG_TX_BUF_SIZE`       |              `512` | FULL 模式下的环形缓冲区大小            |
| `YORULOG_DEFAULT_LEVEL`     |                `4` | 默认日志等级，对应 `TRACE`           |
| `YORULOG_TIMESTAMP`         |                `0` | 是否输出时间戳                     |
| `YORULOG_NOW_MS()`          |    `HAL_GetTick()` | 自定义毫秒时间源                    |
| `YORULOG_CRLF`              |                `1` | 是否使用 `\r\n` 换行              |
| `YORULOG_DROP_NEW_ON_FULL`  |                `0` | 缓冲区满时是否丢弃新数据                |
| `YORULOG_BLOCK_ON_FULL`     |                `1` | 缓冲区满时是否阻塞或刷新                |
| `YORULOG_FORCE_BLOCKING_EW` |                `1` | `ERROR` / `WARN` 是否强制刷新     |
| `YORULOG_DMA_SECTION`       | H7 默认为 `".RAM_D2"` | DMA 缓冲区所在 section           |
| `YORULOG_PREFIX_E/W/I/D/T`  |         `"[E] "` 等 | 自定义日志等级前缀                   |

当前版本仍保留兼容层，旧的 `STLOG_*` 宏，以及 `stlog_*` / `log*` API 仍然可以继续使用。

---

## 行为说明

### MINI 模式

MINI 模式始终使用阻塞式 UART 发送。

它没有环形缓冲，也不会使用 DMA。优点是实现简单、占用极低；缺点是在日志较多时会直接占用 CPU 时间。

适合：

* 资源非常紧张的固件
* 只需要少量启动日志的项目
* 对日志实时性要求不高的量产工程

---

### FULL 模式

FULL 模式使用环形缓冲区。

如果配置了 UART TX DMA，日志会先写入缓冲区，再由 DMA 发送。这样可以减少主循环或业务代码被 UART 线速阻塞的时间。

默认策略优先保证日志完整性，而不是在极端情况下完全不阻塞。也就是说，当缓冲区满时，默认行为是阻塞或刷新，而不是静默丢弃日志。

适合：

* 日志输出较多的调试阶段
* 希望减少 UART 阻塞时间的项目
* 已经配置 UART TX DMA 的工程

---

## 体积测试

测试平台为 STM32H743，使用仓库当前测试工程的 Release 预设。

编译条件：

```text
-Os -g0
--specs=nano.specs
-ffunction-sections -fdata-sections -Wl,--gc-sections
```

### 基线：仅 HAL + USART1

| 资源                       |      占用 |
| ------------------------ | ------: |
| Flash                    | 15536 B |
| DTCMRAM                  |  1856 B |
| RAM_D2                   |     0 B |
| `arm-none-eabi-size` dec |   17376 |

### MINI 模式：仅接入库并调用 `YORULOG_Init()`

| 资源                       |      占用 |    增量 |
| ------------------------ | ------: | ----: |
| Flash                    | 15556 B | +20 B |
| DTCMRAM                  |  1864 B |  +8 B |
| RAM_D2                   |     0 B |  +0 B |
| `arm-none-eabi-size` dec |   17404 |   +28 |

### FULL 模式：接入库并接好 DMA 回调，不运行自测输出

| 资源                       |      占用 |      增量 |
| ------------------------ | ------: | ------: |
| Flash                    | 17092 B | +1556 B |
| DTCMRAM                  |  1888 B |   +32 B |
| RAM_D2                   |   512 B |  +512 B |
| `arm-none-eabi-size` dec |   19476 |   +2100 |

### FULL 模式 + `main.c` 自带自测

| 资源                       |      占用 |  相对基线增量 |
| ------------------------ | ------: | ------: |
| Flash                    | 18532 B | +2996 B |
| DTCMRAM                  |  1888 B |   +32 B |
| RAM_D2                   |   512 B |  +512 B |
| `arm-none-eabi-size` dec |   20916 |   +3540 |

自测字符串和演示逻辑本身会额外增加约 `1440 B` Flash。

MINI 模式不会保留 FULL 模式的 DMA 环形缓冲区，因此在这组测试中 `RAM_D2` 增量为 `+0 B`。

---

## 性能测试

测试平台同样为 STM32H743。

测试条件：

* USART1
* 波特率：115200
* FULL 模式
* 使用 TIM 进行 `1 us` 计时
* 使用 DWT 进行 cycle 计数

### 测试结果

| 测试项              |   数据量 |   入队耗时 |      总耗时 |        吞吐 |
| ---------------- | ----: | -----: | -------: | --------: |
| `raw println x8` | 272 B | 163 us | 23728 us | 11463 B/s |
| `info log x6`    | 228 B | 138 us | 32367 us |  7044 B/s |
| `u32 print x32`  |  54 B | 145 us | 16513 us |  3270 B/s |

### Cycle 计数

| 测试项                     | total | per_call | per_byte |
| ----------------------- | ----: | -------: | -------: |
| `raw println x8 cycles` | 10437 |     1304 |       38 |
| `info log x6 cycles`    |  8790 |     1465 |       38 |
| `u32 print x32 cycles`  |  9281 |      290 |      171 |

### 结论

在这套配置下，Yorulog 本身的入队开销大致是每次调用几十微秒。

字符串输出路径约为 `38 cycles/byte`。整数输出需要进行数字转字符串，因此每字节成本会更高。

端到端耗时主要受 `115200` 串口线速限制，瓶颈不在 logger 本体。

---

## 适用场景

Yorulog 适合以下场景：

* STM32 HAL 项目
* 不想引入 `printf` 的固件
* 需要低成本 UART 日志输出
* 需要在调试阶段快速接入日志
* 希望在 MINI 和 FULL 两种模式之间切换
* STM32H7 项目中需要处理 DMA 可访问内存问题

不太适合以下场景：

* 需要完整格式化字符串能力的项目
* 需要多后端日志输出的项目
* 需要复杂日志过滤、标签、颜色或文件系统记录的项目
* 非 STM32 HAL 工程

---

## 项目定位

Yorulog 不是一个完整的通用日志框架。

它更像是一个适合直接塞进 STM32 工程里的小工具：简单、可控、占用低，主要解决“我需要一点可靠的 UART 日志，但不想把 `printf` 搬进来”的问题。
