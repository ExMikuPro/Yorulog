# Yorulog

Yorulog 是一个面向 **STM32 HAL** 的轻量级 UART 日志库。

它采用单头文件设计，不依赖 `printf`，也不引入完整的格式化解析器。它的目标很明确：在尽量低的 Flash、RAM 和栈开销下，为 STM32 项目提供足够好用、足够可控的串口日志输出能力。

Yorulog 适合那些资源紧张、但仍然需要基础调试输出的固件工程。


---

> **Yoru 系列**
>
> 一组面向 STM32 HAL 的轻量级工具库。各库可独立使用，也可以组合使用。
>
> | 库 | 定位 |
> | --- | --- |
> | [Yorulog](https://github.com/ExMikuPro/Yorulog) | 轻量级 UART 日志库 |
> | [Yorush](https://github.com/ExMikuPro/Yorush) | 轻量级 UART Shell / 命令解析器 |
> | [Yorunvm](https://github.com/ExMikuPro/Yorunvm) | STM32 片上 NVM / Flash / EEPROM 访问辅助库 |
> | [Yorukv](https://github.com/ExMikuPro/Yorukv) | 轻量级 KV 配置库 |
> | [Yorubench](https://github.com/ExMikuPro/Yorubench) | 轻量级性能测量库 |
> | [Yoruassert](https://github.com/ExMikuPro/Yoruassert.git) | 轻量级断言辅助库 |

---

## 特性

- **单头文件接入**：只需要引入 `yorulog.h`
- **面向 STM32 HAL UART**：直接使用 `UART_HandleTypeDef`
- **不使用 `printf`**：避免额外拉入较重的格式化运行时
- **两种工作模式**：
  - `MINI`：阻塞式 UART 直发，占用最低
  - `FULL`：环形缓冲，支持 UART TX DMA，必要时回退阻塞发送
- **日志等级**：
  - `ERROR`
  - `WARN`
  - `INFO`
  - `DEBUG`
  - `TRACE`
- **等级前缀**：
  - `[E]`
  - `[W]`
  - `[I]`
  - `[D]`
  - `[T]`
- **支持附加标签**：
  - `[net]`
  - `[ui]`
  - `[sensor]`
- **可选时间戳**
- **常见类型自动输出**：
  - 字符串
  - 字符
  - 有符号整数
  - 无符号整数
  - 指针
- **支持长文本输出路径**
- **可自动探测 STM32 HAL 头文件**
- **STM32H7 默认支持 DMA 可访问缓冲区放置**

---

## 项目定位

Yorulog 不是一个完整的通用日志框架，也不是 `printf` 的完整替代品。

它更像是一个可以直接放进 STM32 工程里的小工具：  
在不引入重型格式化能力的前提下，提供基本可靠、低占用、容易控制的 UART 日志输出。

适合：

- STM32 HAL 项目
- 不想引入 `printf` 的固件
- 小容量 Flash / RAM 的 MCU
- 需要快速接入 UART 调试日志的项目
- 希望在阻塞日志和 DMA 日志之间切换的工程
- STM32H7 / F7 这类需要注意 DMA cache 和内存区域的项目

不适合：

- 需要完整格式化字符串能力的项目
- 需要多后端日志输出的项目
- 需要日志标签、颜色、文件系统记录等复杂功能的项目
- 非 STM32 HAL 工程

---

## 为什么不用 `printf`

在小型 MCU 上，`printf` 很方便，但代价也明显。

它通常会引入额外的格式化运行时，导致：

- Flash 占用增加
- 栈占用增加
- 链接结果不够直观
- 打开浮点格式化后体积进一步膨胀
- 对只需要简单日志的工程来说过于重

Yorulog 的做法是绕过格式化解析器，只保留嵌入式调试里最常见的输出能力。这样实现更简单，代码体积也更可控。

---

## 快速开始

### 1. 添加头文件

把 `yorulog.h` 放入工程目录，例如：

```text
Core/Yorulog/yorulog.h
```

然后在需要使用日志的文件中包含它：

```c
#include "yorulog.h"
```

---

### 2. 在一个 `.c` 文件中定义全局实现

Yorulog 是单头文件库。你需要在且只在一个 `.c` 文件中定义 `YORULOG_DEFINE_GLOBALS`。

建议放在 `main.c` 或单独的 `yorulog_port.c` 中：

```c
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

其他 `.c` 文件只需要正常包含：

```c
#include "yorulog.h"
```

---

### 3. 初始化 UART

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

## 选择工作模式

Yorulog 提供 `MINI` 和 `FULL` 两种模式。

| 模式 | 发送方式 | RAM 占用 | DMA | 适合场景 |
| --- | --- | ---: | --- | --- |
| `MINI` | 阻塞式 UART 发送 | 最低 | 不使用 | 极小工程、少量启动日志、量产保底日志 |
| `FULL` | 环形缓冲 + 可选 DMA | 较高 | 支持 UART TX DMA | 调试阶段、日志较多、需要减少业务代码阻塞 |

---

### MINI 模式

`MINI` 模式始终使用阻塞式 UART 发送。

它没有环形缓冲，也不会使用 DMA。优点是实现简单、占用极低；缺点是在日志较多时，业务代码会直接等待 UART 发送完成。

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 1

#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

适合：

- Flash / RAM 极小的 MCU
- 只需要少量启动日志
- 不希望引入任何额外缓冲区
- 可以接受日志输出短暂阻塞主循环

---

### FULL 模式

`FULL` 模式使用环形缓冲区。

如果 UART 句柄已经配置了 TX DMA，Yorulog 会自动使用 DMA 发送；如果没有配置 TX DMA，则会自动回退到阻塞发送。

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1

#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

适合：

- 调试阶段日志较多
- 希望减少 UART 线速阻塞
- 已经在 CubeMX / HAL 中配置 UART TX DMA
- 需要配合 shell、帮助菜单、长文本输出等场景

默认策略更偏向保证日志完整性。  
当缓冲区满时，默认行为是阻塞或刷新，而不是静默丢弃新日志。

---

## FULL 模式与 DMA

在 `FULL` 模式下，如果满足以下条件，Yorulog 会使用 UART TX DMA：

```c
huart->hdmatx != NULL
```

你需要把 HAL 的 UART 发送完成回调转发给 Yorulog：

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}
```

如果没有配置 TX DMA，`FULL` 模式仍然可以工作，只是发送方式会回退到阻塞发送。

---

## STM32H7 / F7 注意事项

STM32H7 的内存结构比较特殊。很多工程会把 `.data` / `.bss` 默认放在 DTCM 中，但 **DMA 不能访问 DTCM**。

因此，Yorulog 在 STM32H7 上默认会把 DMA 发送缓冲区放入 DMA 可访问的 section。

默认 section 名称：

```c
#define YORULOG_DMA_SECTION ".RAM_D2"
```

你需要在链接脚本中把这个 section 放到 DMA 可访问的 RAM 中，例如：

```ld
.RAM_D2 (NOLOAD) :
{
  . = ALIGN(32);
  KEEP(*(.RAM_D2))
  KEEP(*(.RAM_D2*))
  . = ALIGN(32);
} >RAM_D2
```

如果工程启用了 D-Cache，还需要注意 DMA 缓冲区一致性。

在 H7 / F7 这类 Cortex-M7 目标上，Yorulog 默认会在启动 DMA TX 前执行一次 cache clean：

```c
#define YORULOG_DMA_CACHE_CLEAN 1
#define YORULOG_DMA_CACHE_LINE_SIZE 32
```

更稳妥的做法是使用 MPU，把 DMA 缓冲区所在区域配置为 non-cacheable。

---

## API

### 初始化

```c
YORULOG_Init(&huart1);
```

---

### 设置日志等级

```c
YORULOG_SetLevel(YORULOG_LEVEL_INFO);
```

等级过滤规则：只有严重程度不低于当前等级的日志会输出。

例如设置为 `YORULOG_LEVEL_INFO` 后：

- `ERROR` 会输出
- `WARN` 会输出
- `INFO` 会输出
- `DEBUG` 会被忽略
- `TRACE` 会被忽略

---

### 日志输出

```c
YORULOG_LogError("fatal");
YORULOG_LogWarn("warn");
YORULOG_LogInfo("info");
YORULOG_LogDebug("debug");
YORULOG_LogTrace("trace");
```

日志宏接收一个参数，并自动识别常见类型：

| 类型 | 示例 |
| --- | --- |
| `const char*` / `char*` | `YORULOG_LogInfo("boot");` |
| `char` | `YORULOG_LogDebug('A');` |
| 有符号整数 | `YORULOG_LogDebug(-123);` |
| 无符号整数 | `YORULOG_LogDebug(123u);` |
| 指针 | `YORULOG_LogWarn((void*)0x20000000);` |

---

### 带标签的日志输出

如果你希望在日志行里额外带一个轻量级模块名或功能标记，可以使用 `Tag` 系列宏：

```c
YORULOG_LogTag("boot", "start");
YORULOG_LogInfoTag("net", "link up");
YORULOG_LogWarnTag("sensor", 3);
YORULOG_LogErrorTag("flash", (void*)0x08020000);
```

典型输出类似：

```text
[boot] start
[I] [net] link up
[W] [sensor] 3
[E] [flash] 0x08020000
```

如果 `tag` 是 `NULL` 或空字符串，这一段附加标签会被自动跳过。

---

### 原始输出

```c
YORULOG_Print("x=");
YORULOG_Print(123);
YORULOG_Println("done");
```

`YORULOG_Print()` 不会自动换行。  
`YORULOG_Println()` 会在输出后追加换行。

另外还提供了 `YORULOG_PrintRaw()` 和 `YORULOG_PrintRawln()` 这组更显式的原始输出别名：

```c
YORULOG_PrintRaw("AT+RST");
YORULOG_PrintRawln("\r");
```

它们当前与 `YORULOG_Print()` / `YORULOG_Println()` 走的是同一条输出路径，但在调用处更容易表达“这里只做纯输出，不带日志等级前缀”的语义。

如果你希望在纯文本整行输出后追加调用位置，可以开启：

```c
#define YORULOG_TRACE_PRINT_CALLSITE 1
```

示例：

```text
done (main.c:123)
```

---

### 长文本输出

如果需要输出帮助菜单、shell 提示、较长说明文本，建议使用长文本 API：

```c
YORULOG_PrintLong("help: ");
YORULOG_PrintLongln("show all commands");
```

在 `FULL` 模式下，长文本 API 会先刷新已有缓冲日志，然后对这段长文本使用阻塞式 UART 发送。  
在 `MINI` 模式下，它的行为和普通阻塞发送一致。

这个路径更适合需要保证“整行输出完整性”的场景。

---

### 手动刷新

```c
YORULOG_Flush();
```

在 `FULL` 模式下，可以使用它主动刷新缓冲区内容。

---

### 可选接入 Yoruassert

Yorulog 可以可选接入 Yoruassert，用于内部参数和状态检查：

```c
#define YORULOG_USE_YORUASSERT 1
#define YORULOG_YORUASSERT_HEADER "../Yoruassert/yoruassert.h"
```

如果你希望非法使用日志库时直接触发断言，可以保留：

```c
#define YORULOG_ASSERT_ON_ERROR 1
```

典型场景包括：`YORULOG_Init(NULL)`、未初始化就输出、或传入非法日志等级。

---

### 可选调用位置尾注

如果你希望完整日志接口在行尾追加调用位置，可以开启：

```c
#define YORULOG_TRACE_LOG_CALLSITE 1
```

示例：

```text
[I] boot ok (main.c:123)
```

---

## 配置宏

所有配置宏都需要在包含 `yorulog.h` 之前定义。

| 宏 | 默认值 | 说明 |
| --- | ---: | --- |
| `YORULOG_ENABLE` | `1` | 总开关 |
| `YORULOG_MINI` | `0` | `1` 为 MINI 模式，`0` 为 FULL 模式 |
| `YORULOG_TX_BUF_SIZE` | `512` | FULL 模式下的环形缓冲区大小 |
| `YORULOG_DEFAULT_LEVEL` | `4` | 默认日志等级，对应 `TRACE` |
| `YORULOG_TIMESTAMP` | `0` | 是否输出时间戳 |
| `YORULOG_NOW_MS()` | `HAL_GetTick()` | 自定义毫秒时间源 |
| `YORULOG_CRLF` | `1` | 是否使用 `\r\n` 换行 |
| `YORULOG_DROP_NEW_ON_FULL` | `0` | 缓冲区满时是否丢弃新数据 |
| `YORULOG_BLOCK_ON_FULL` | `1` | 缓冲区满时是否阻塞或刷新 |
| `YORULOG_FORCE_BLOCKING_EW` | `1` | `ERROR` / `WARN` 是否强制刷新 |
| `YORULOG_USE_YORUASSERT` | `0` | 是否启用可选的 Yoruassert 联动 |
| `YORULOG_YORUASSERT_HEADER` | `"../Yoruassert/yoruassert.h"` | 自定义 Yoruassert 头文件路径 |
| `YORULOG_ASSERT_ON_ERROR` | `1` | 在启用 Yoruassert 时，非法日志调用是否触发断言 |
| `YORULOG_TRACE_PRINT_CALLSITE` | `0` | 是否给 `Println` / `PrintRawln` 追加 `(file:line)` |
| `YORULOG_TRACE_LOG_CALLSITE` | `0` | 是否给 `LogTag` / `LogXxx` / `LogXxxTag` 追加 `(file:line)` |
| `YORULOG_DMA_SECTION` | H7 默认 `".RAM_D2"` | DMA 缓冲区所在 section |
| `YORULOG_DMA_CACHE_CLEAN` | H7 / F7 类目标默认启用 | DMA TX 前是否执行 D-Cache clean |
| `YORULOG_DMA_CACHE_LINE_SIZE` | `32` | cache clean 对齐使用的 cache line 大小 |
| `YORULOG_PREFIX_E/W/I/D/T` | `"[E] "` 等 | 自定义日志等级前缀 |

当前配置项统一使用 `YORULOG_*` 前缀。  
旧的 `STLOG_*` 配置宏不再保留。

---

## 行为说明

### 缓冲区满时

在 `FULL` 模式下，Yorulog 默认优先保证日志完整性。  
当环形缓冲区写满时，默认行为是阻塞或刷新，而不是直接丢弃新数据。

如果你更关心业务代码不能被日志阻塞，可以调整：

```c
#define YORULOG_DROP_NEW_ON_FULL 1
#define YORULOG_BLOCK_ON_FULL 0
```

这样在缓冲区满时，新日志会被丢弃。

---

### ERROR / WARN 刷新策略

默认情况下，`ERROR` 和 `WARN` 会强制刷新，尽量减少关键日志留在缓冲区中的概率：

```c
#define YORULOG_FORCE_BLOCKING_EW 1
```

如果你希望所有日志都尽量走缓冲路径，可以关闭它：

```c
#define YORULOG_FORCE_BLOCKING_EW 0
```

---

### 与 shell / 命令帮助配合

如果要配合 Yorush 这类 shell 中间件使用：

- `MINI` 可以正常工作，但长帮助文本会完全阻塞发送，主循环可能短暂停顿。
- `FULL` 会占用额外 RAM，但在日志较多或命令输出较长时更平衡。
- 对帮助菜单、命令列表等长文本，建议使用 `YORULOG_PrintLong()` / `YORULOG_PrintLongln()`。

---

## 体积测试

测试平台：

- MCU：STM32H743
- 工程：仓库当前测试工程
- 配置：Release 预设

编译条件：

```text
-Os -g0
--specs=nano.specs
-ffunction-sections -fdata-sections -Wl,--gc-sections
```

### 基线：仅 HAL + USART1

| 资源 | 占用 |
| --- | ---: |
| Flash | 15536 B |
| DTCMRAM | 1856 B |
| RAM_D2 | 0 B |
| `arm-none-eabi-size` dec | 17376 |

### MINI 模式：仅接入库并调用 `YORULOG_Init()`

| 资源 | 占用 | 增量 |
| --- | ---: | ---: |
| Flash | 15556 B | +20 B |
| DTCMRAM | 1864 B | +8 B |
| RAM_D2 | 0 B | +0 B |
| `arm-none-eabi-size` dec | 17404 | +28 |

### FULL 模式：接入库并接好 DMA 回调，不运行自测输出

| 资源 | 占用 | 增量 |
| --- | ---: | ---: |
| Flash | 17092 B | +1556 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 19476 | +2100 |

### FULL 模式 + `main.c` 自带自测

| 资源 | 占用 | 相对基线增量 |
| --- | ---: | ---: |
| Flash | 18532 B | +2996 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 20916 | +3540 |

自测字符串和演示逻辑本身会额外增加约 `1440 B` Flash。

`MINI` 模式不会保留 `FULL` 模式的 DMA 环形缓冲区，因此在这组测试中 `RAM_D2` 增量为 `+0 B`。

---

## 性能测试

测试平台同样为 STM32H743。

测试条件：

- USART1
- 波特率：115200
- `FULL` 模式
- 使用 TIM 进行 `1 us` 计时
- 使用 DWT 进行 cycle 计数

### 测试结果

| 测试项 | 数据量 | 入队耗时 | 总耗时 | 吞吐 |
| --- | ---: | ---: | ---: | ---: |
| `raw println x8` | 272 B | 163 us | 23728 us | 11463 B/s |
| `info log x6` | 228 B | 138 us | 32367 us | 7044 B/s |
| `u32 print x32` | 54 B | 145 us | 16513 us | 3270 B/s |

### Cycle 计数

| 测试项 | total | per_call | per_byte |
| --- | ---: | ---: | ---: |
| `raw println x8 cycles` | 10437 | 1304 | 38 |
| `info log x6 cycles` | 8790 | 1465 | 38 |
| `u32 print x32 cycles` | 9281 | 290 | 171 |

### 结论

在这套配置下，Yorulog 的入队开销大致是每次调用几十微秒。

字符串输出路径约为 `38 cycles/byte`。  
整数输出需要数字转字符串，因此每字节成本更高。

端到端耗时主要受 `115200` 串口线速限制，瓶颈不在 logger 本体。

---

## 推荐配置

### 极限低占用

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 0
```

适合小 Flash / 小 RAM MCU，或者只需要少量启动日志的项目。

---

### 常规调试

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1
```

适合大多数调试阶段。

---

### 更偏向不阻塞业务代码

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_DROP_NEW_ON_FULL 1
#define YORULOG_BLOCK_ON_FULL 0
#define YORULOG_FORCE_BLOCKING_EW 0
```

适合日志不能影响实时任务的场景。代价是缓冲区满时可能丢日志。

---

## 开源协议

本项目使用 MIT License 开源。

详细内容见 [LICENSE](./LICENSE)。
