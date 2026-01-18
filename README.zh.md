# easySTlogger

一个 **极简、单头文件** 的 STM32（HAL）串口日志模块，专为 **极低 Flash / RAM 占用** 设计，支持 **日志分级、前缀、可选时间戳**，并提供可选的 **FULL 模式（环形缓冲 + 自动 DMA 发送）**。

> 不使用 `printf`  
> 不引入格式化解析  
> 不依赖沉重的标准库  
>  
> 只需一个头文件即可使用。

---

## 特性一览

- ✅ **单头文件**（`stlog.h`），即拷即用
- ✅ **仅依赖 HAL + UART**，适用于各类 STM32
- ✅ **两种工作模式**
  - **MINI 模式**：极低 Flash / RAM，占用最小，阻塞直发
  - **FULL 模式**：环形缓冲 + **自动 DMA TX**（存在 `huart->hdmatx` 时）
- ✅ **日志分级**：`ERROR / WARN / INFO / DEBUG / TRACE`
- ✅ **分级前缀**：`[E] [W] [I] [D] [T]`
- ✅ **可选时间戳**：`[HAL_GetTick()]`（毫秒）
- ✅ **类型自适应 print / println**
  - 基于 C11 `_Generic`
  - 支持：字符串、字符、整数、指针
- ✅ **链接器友好**
  - 强烈受益于  
    `-ffunction-sections -fdata-sections -Wl,--gc-sections`

---

## 为什么不用 `printf`？

在嵌入式系统（尤其是小 Flash MCU）中，`printf` 往往会：

- 引入大量格式化代码（`vfprintf` / `dtoa` 等）
- Flash 占用不可控（动辄数 KB 甚至十几 KB）
- 栈占用大，不适合中断环境
- 很难裁剪和模块化

**easySTlogger 完全绕开格式化解析**，只做“最小必要输出”，体积与行为都高度可控。

在 STM32G0（G031，-O3）上的实测：
- **MINI 模式：Flash 增量 < 400B**
- **FULL 模式：Flash 增量 ~1.2KB**

---

## 体积对比（实测数据）

以下数据 **全部来自真实测试结果**，编译条件如下：

- `-O3`
- `--specs=nano.specs`
- `-ffunction-sections -fdata-sections -Wl,--gc-sections`
- 目标芯片：**STM32G031（64 KB Flash / 8 KB RAM）**

### 基线（仅 HAL + USART1）

| 资源 | 占用 |
|----|----|
| RAM | 1728 B |
| Flash | 7904 B |

---

### 启用 MINI 模式

| 资源 | 占用 | 增量 |
|----|----|----|
| RAM | 1736 B | **+8 B** |
| Flash | 8288 B | **+384 B** |

> MINI 模式的体积代价几乎可以忽略，  
> 可安全常驻于量产固件中。

---

### 启用 FULL 模式（环形缓冲 + 自动 DMA）

| 资源 | 占用 | 增量 |
|----|----|----|
| RAM | 2264 B | **+536 B** |
| Flash | 9096 B | **+1192 B** |

> RAM 增量主要来自环形缓冲区，  
> Flash 增量即完整日志系统的全部成本。

---

### 与 `printf` 的对比

STM32 工程中常见 `printf` 串口调试的体积代价：

| 方案 | Flash 增量 |
|----|-----------|
| `printf("%d")` | +3 KB ~ +8 KB |
| `printf("%f")` | +10 KB ~ +20 KB |
| **easySTlogger（MINI）** | **+0.38 KB** |
| **easySTlogger（FULL）** | **+1.2 KB** |

easySTlogger 不会引入 `vfprintf`、浮点格式化或不可控的栈开销。


## 快速上手

### 1️⃣ 添加 `stlog.h`

将 `stlog.h` 放入工程，例如：

```
/Core/Inc/stlog.h
```

并在代码中包含：

```c
#include "stlog.h"
```

---

### 2️⃣ 通过宏选择模式（必须在 include 之前）

#### MINI 模式（极致轻量）

```c
#define STLOG_MINI 1
#define STLOG_TIMESTAMP 1   // 可选
#include "stlog.h"
```

#### FULL 模式（环形缓冲 + DMA）

```c
#define STLOG_MINI 0
#define STLOG_TX_BUF_SIZE 512
#define STLOG_TIMESTAMP 1
#include "stlog.h"
```

---

### 3️⃣ 使用 USART 初始化

```c
extern UART_HandleTypeDef huart1;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_USART1_UART_Init();

  stlog_init(&huart1);
  log_set_level(LOG_DEBUG);

  logi("boot");
  logd(123);
  logw((void*)0x20000000);

  while (1) { }
}
```

---

## FULL 模式：DMA 发送完成回调（必须）

当 `huart->hdmatx != NULL` 时，FULL 模式会自动使用 DMA 发送。  
你需要在 HAL 回调中转发：

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    stlog_on_tx_cplt(huart);
}
```

如果未配置 DMA TX，FULL 模式会自动回退为阻塞发送。

---

## API 使用说明

### 带前缀与换行的日志接口

```c
loge("fatal");
logw("warn");
logi("info");
logd("debug");
logt("trace");
```

每个接口 **只接收一个参数**（类型自动识别）：

- `const char* / char*`
- `char`
- 整数类型
- 指针

---

### 无前缀的原始输出

```c
print("x=");
print(123);
println("done");
```

---

### 动态设置日志等级

```c
log_set_level(LOG_INFO);  // DEBUG / TRACE 将被过滤
```

---

## 配置宏一览

在 `#include "stlog.h"` 之前定义：

| 宏 | 默认值 | 说明 |
|---|---:|---|
| `STLOG_ENABLE` | `1` | 总开关 |
| `STLOG_MINI` | `0` | `1` = MINI，`0` = FULL |
| `STLOG_TX_BUF_SIZE` | `512` | FULL 模式环形缓冲大小 |
| `STLOG_DROP_NEW_ON_FULL` | `1` | FULL 模式满时丢弃新数据 |
| `STLOG_DEFAULT_LEVEL` | `LOG_INFO` | 默认日志等级 |
| `STLOG_TIMESTAMP` | `0` | 启用时间戳 |
| `STLOG_NOW_MS()` | `HAL_GetTick()` | 自定义时间源 |
| `STLOG_CRLF` | `1` | 使用 `\r\n` |
| `STLOG_PREFIX_E/W/I/D/T` | `"[E] "` 等 | 日志前缀 |
| `STLOG_FORCE_BLOCKING_EW` | `1` | ERROR/WARN 强制阻塞刷新 |

---

## 使用注意事项

### MINI 模式
- 所有输出均为 **阻塞式 UART 发送**
- 在高频日志或中断中可能会明显拖慢系统（这是预期行为）
- 适合量产 / 极限资源场景

### FULL 模式
- 使用环形缓冲，尽量避免阻塞
- 自动识别 DMA TX
- **STM32H7 + D-Cache** 场景下，需注意 DMA 缓冲区的 Cache 一致性（建议放在 non-cacheable 区域或使用 MPU）

---

## License

MIT（或根据你的项目选择）

---

## 项目命名建议

推荐：**easySTlogger**

备选：
- **tinySTlog**
- **stloglite**
- **stm32-uart-log**
- **microSTlogger**
