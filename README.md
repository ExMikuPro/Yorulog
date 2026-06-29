# Yorulog

Yorulog is a lightweight UART logger for STM32 HAL.

It is distributed as a single header, does not depend on `printf`, and does not pull in an extra format parser. The goal is to keep Flash, RAM, and stack usage as low as possible while still providing the logging features commonly needed in embedded development, such as log levels, level prefixes, optional timestamps, and DMA-based asynchronous transmission.

It is a good fit for STM32 projects that are relatively resource-constrained but still need basic debug output.

---

## Key Features

* Single header: just include `yorulog.h`
* Based on STM32 HAL UART
* No `printf`
* No heavy stdlib formatting dependency
* Supports two operating modes:

  * MINI: blocking direct transmit, smallest footprint
  * FULL: ring buffer with optional automatic UART TX DMA
* Supports log levels:

  * `ERROR`
  * `WARN`
  * `INFO`
  * `DEBUG`
  * `TRACE`
* Supports level prefixes:

  * `[E]`
  * `[W]`
  * `[I]`
  * `[D]`
  * `[T]`
* Supports optional timestamps
* Supports `YORULOG_Print()` / `YORULOG_Println()` with automatic handling of common types
* Can auto-detect the STM32 HAL header
* Enables DMA-accessible buffer support by default on STM32H7

---

## Why not use `printf`

On small MCUs, `printf` is convenient, but the tradeoff is often obvious.

It usually pulls in extra formatting runtime code, increases Flash usage, increases stack usage, and makes final code size harder to predict. For firmware that only needs simple logging, that overhead is not always worth it.

Yorulog avoids format parsing and only provides direct output for common types. That keeps the implementation simpler and the runtime behavior more predictable.

---

## Quick Start

### 1. Add the header

Place `yorulog.h` in your project, for example:

```text
/Core/Yorulog/yorulog.h
```

### 2. Define the global implementation in one `.c` file

In exactly one `.c` file, define the following before including the header:

```c
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

In all other `.c` files, just include it normally:

```c
#include "yorulog.h"
```

---

## Select the operating mode

### MINI mode

MINI mode uses blocking UART transmission. It has the simplest logic and the lowest footprint, so it is suitable for projects with very tight resource limits or only a small amount of logging.

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

### FULL mode

FULL mode uses a ring buffer. If TX DMA is configured in the UART handle, Yorulog will automatically use DMA for transmission. If DMA is not configured, it will fall back to blocking transmit.

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

---

## Basic Usage

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

## FULL Mode and DMA

In FULL mode, if `huart->hdmatx != NULL`, Yorulog will automatically use DMA TX.

The HAL transmit-complete callback must be forwarded to Yorulog:

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}
```

If TX DMA is not configured, FULL mode still works and automatically falls back to blocking transmission.

---

## STM32H7 Notes

STM32H7 has a special memory layout. In many projects, the default `.data` / `.bss` sections are placed in DTCM, but DMA cannot access DTCM.

For that reason, Yorulog places the DMA transmit buffer into a DMA-accessible section by default on STM32H7.

The default section name is:

```c
#define YORULOG_DMA_SECTION ".RAM_D2"
```

Your linker script needs to place that section into DMA-accessible RAM, for example:

```ld
.RAM_D2 (NOLOAD) :
{
  . = ALIGN(32);
  KEEP(*(.RAM_D2))
  KEEP(*(.RAM_D2*))
  . = ALIGN(32);
} >RAM_D2
```

If D-Cache is enabled in the project, you also need to consider DMA buffer coherency.

The current release provides a default cache-clean step before starting DMA TX on H7/F7-class targets:

```c
#define YORULOG_DMA_CACHE_CLEAN 1
#define YORULOG_DMA_CACHE_LINE_SIZE 32
```

You can still use MPU to mark the DMA region as non-cacheable, which is often the cleanest setup on STM32H7.

---

## API

### Logging output

```c
YORULOG_LogError("fatal");
YORULOG_LogWarn("warn");
YORULOG_LogInfo("info");
YORULOG_LogDebug("debug");
YORULOG_LogTrace("trace");
```

The logging macros take one argument and automatically handle common types:

* `const char*`
* `char*`
* `char`
* signed integers
* unsigned integers
* pointers

---

### Raw output

```c
YORULOG_Print("x=");
YORULOG_Print(123);
YORULOG_Println("done");
```

`YORULOG_Print()` does not append a newline.
`YORULOG_Println()` appends a newline after the output.

For long menu text, shell help, or other output where line completeness matters more than non-blocking behavior, use the long-text path:

```c
YORULOG_PrintLong("help: ");
YORULOG_PrintLongln("show all commands");
```

In FULL mode, the long-text APIs flush pending buffered logs first and then use blocking UART transmit for the long text itself. In MINI mode, they behave like the normal blocking path.

---

### Set log level

```c
YORULOG_SetLevel(YORULOG_LEVEL_INFO);
```

Only logs at or above the active level are printed.

For example, if the level is set to `YORULOG_LEVEL_INFO`, then `ERROR`, `WARN`, and `INFO` are printed, while `DEBUG` and `TRACE` are ignored.

---

### Manual flush

```c
YORULOG_Flush();
```

In FULL mode, this can be used to actively flush the ring buffer.

---

## Configuration Macros

All configuration macros must be defined before including `yorulog.h`.

| Macro | Default | Description |
| --- | ---: | --- |
| `YORULOG_ENABLE` | `1` | master enable switch |
| `YORULOG_MINI` | `0` | `1` = MINI mode, `0` = FULL mode |
| `YORULOG_TX_BUF_SIZE` | `512` | ring buffer size in FULL mode |
| `YORULOG_DEFAULT_LEVEL` | `4` | default log level, which means `TRACE` |
| `YORULOG_TIMESTAMP` | `0` | whether to output timestamps |
| `YORULOG_NOW_MS()` | `HAL_GetTick()` | custom millisecond time source |
| `YORULOG_CRLF` | `1` | whether to use `\r\n` |
| `YORULOG_DROP_NEW_ON_FULL` | `0` | whether to drop new data when the buffer is full |
| `YORULOG_BLOCK_ON_FULL` | `1` | whether to block or flush when the buffer is full |
| `YORULOG_FORCE_BLOCKING_EW` | `1` | whether `ERROR` / `WARN` should force a flush |
| `YORULOG_DMA_SECTION` | `".RAM_D2"` on H7 | section used for the DMA buffer |
| `YORULOG_DMA_CACHE_CLEAN` | `1` on H7/F7-class targets | clean D-Cache before DMA TX |
| `YORULOG_DMA_CACHE_LINE_SIZE` | `32` | cache line size used for DMA clean alignment |
| `YORULOG_PREFIX_E/W/I/D/T` | `"[E] "` etc | custom level prefixes |

The current release still keeps a compatibility layer, so the old `STLOG_*` macros and `stlog_*` / `log*` APIs can still be used.

---

## Behavior Notes

### MINI mode

MINI mode always uses blocking UART transmission.

It has no ring buffer and does not use DMA. The advantage is a very small footprint and simple behavior; the downside is that a large amount of logging directly consumes CPU time.

Suitable for:

* very resource-constrained firmware
* projects that only need a small amount of boot-time logging
* production firmware where log timing is not critical

---

### FULL mode

FULL mode uses a ring buffer.

If UART TX DMA is configured, logs are first written into the buffer and then transmitted by DMA. This reduces the amount of time that the main loop or business logic is blocked by UART line speed.

The default strategy prioritizes log completeness over being completely non-blocking in extreme conditions. In other words, when the buffer is full, the default behavior is to block or flush rather than silently drop logs.

Suitable for:

* debug phases with heavier log output
* projects that want to reduce UART blocking time
* firmware that already has UART TX DMA configured
* shell-style features such as Yorush, command help, and longer menu text

When used together with Yorush or other shell-style middleware:

* `MINI` still works correctly, but long help text is fully blocking and may pause the main loop briefly.
* `FULL` is usually the better balance if the project can spare the extra RAM.

---

## Size Measurements

The test platform is STM32H743 using the current repository test project and the Release preset.

Build flags:

```text
-Os -g0
--specs=nano.specs
-ffunction-sections -fdata-sections -Wl,--gc-sections
```

### Baseline: HAL + USART1 only

| Resource | Usage |
| --- | ---: |
| Flash | 15536 B |
| DTCMRAM | 1856 B |
| RAM_D2 | 0 B |
| `arm-none-eabi-size` dec | 17376 |

### MINI mode: library integrated and only `YORULOG_Init()` called

| Resource | Usage | Increment |
| --- | ---: | ---: |
| Flash | 15556 B | +20 B |
| DTCMRAM | 1864 B | +8 B |
| RAM_D2 | 0 B | +0 B |
| `arm-none-eabi-size` dec | 17404 | +28 |

### FULL mode: library integrated with DMA callback wired, but no self-test output

| Resource | Usage | Increment |
| --- | ---: | ---: |
| Flash | 17092 B | +1556 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 19476 | +2100 |

### FULL mode + built-in self-test in `main.c`

| Resource | Usage | Increment vs baseline |
| --- | ---: | ---: |
| Flash | 18532 B | +2996 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 20916 | +3540 |

The self-test strings and demo logic add about `1440 B` of extra Flash by themselves.

MINI mode does not keep the FULL-mode DMA ring buffer, so `RAM_D2` remains at `+0 B` in this measurement.

---

## Performance Notes

Measured on the same STM32H743 test project with `USART1 @ 115200` in FULL mode:

* `raw println x8`: `272 B`, `enqueue_us=163`, `total_us=23728`, `throughput_Bps=11463`
* `raw println x8 cycles`: `total=10437`, `per_call=1304`, `per_byte=38`
* `info log x6`: `228 B`, `enqueue_us=138`, `total_us=32367`, `throughput_Bps=7044`
* `info log x6 cycles`: `total=8790`, `per_call=1465`, `per_byte=38`
* `u32 print x32`: `54 B`, `enqueue_us=145`, `total_us=16513`, `throughput_Bps=3270`
* `u32 print x32 cycles`: `total=9281`, `per_call=290`, `per_byte=171`

Interpretation:

* On this setup, the logger's own enqueue overhead is roughly in the **tens of microseconds per call**.
* In this benchmark, the string path is about `38 cycles/byte`, while integer-to-string output is noticeably more expensive.
* End-to-end throughput is mainly limited by the `115200` UART line rate, not by the logger itself.
* The example test project now includes both a TIM-based `1 us` timing benchmark and a DWT cycle counter benchmark, which makes it easier to compare time-in-us and CPU-cycle cost across STM32 targets.

---

## License

This project is licensed under the MIT License.

See the [LICENSE](./LICENSE) file for details.
