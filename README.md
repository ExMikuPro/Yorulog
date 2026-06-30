# Yorulog

Yorulog is a lightweight UART logging library for **STM32 HAL**.

It is distributed as a single header, does not depend on `printf`, and does not pull in a full format parser. Its goal is straightforward: provide UART logging that is usable and controllable enough for STM32 projects while keeping Flash, RAM, and stack overhead as low as possible.

Yorulog is a good fit for firmware projects that are resource-constrained but still need basic debug output.

---

> **Yoru Series**
>
> A family of lightweight utility libraries for STM32 HAL. Each library can be used independently or combined as needed.
>
> | Library | Role |
> | --- | --- |
> | [Yorulog](https://github.com/ExMikuPro/Yorulog) | Lightweight UART logger |
> | [Yorush](https://github.com/ExMikuPro/Yorush) | Lightweight UART shell / command parser |
> | [Yorunvm](https://github.com/ExMikuPro/Yorunvm) | STM32 on-chip NVM / Flash / EEPROM access helper |
> | [Yorukv](https://github.com/ExMikuPro/Yorukv) | Lightweight KV configuration library |
> | [Yorubench](https://github.com/ExMikuPro/Yorubench) | Lightweight performance measurement library |
> | [Yoruassert](https://github.com/ExMikuPro/Yoruassert.git) | Lightweight assertion helper |

---

## Features

- **Single-header integration**: just include `yorulog.h`
- **Designed for STM32 HAL UART**: works directly with `UART_HandleTypeDef`
- **No `printf`**: avoids pulling in a heavier formatting runtime
- **Two operating modes**:
  - `MINI`: blocking direct UART transmit with the smallest footprint
  - `FULL`: ring buffer with UART TX DMA support and blocking fallback when needed
- **Log levels**:
  - `ERROR`
  - `WARN`
  - `INFO`
  - `DEBUG`
  - `TRACE`
- **Level prefixes**:
  - `[E]`
  - `[W]`
  - `[I]`
  - `[D]`
  - `[T]`
- **Optional extra tags**:
  - `[net]`
  - `[ui]`
  - `[sensor]`
- **Optional timestamps**
- **Automatic output for common types**:
  - string
  - character
  - signed integer
  - unsigned integer
  - pointer
- **Long-text output path**
- **Automatic STM32 HAL header detection**
- **DMA-accessible buffer placement enabled by default on STM32H7**

---

## Project Positioning

Yorulog is not a full general-purpose logging framework, and it is not a full replacement for `printf`.

It is closer to a small utility that can be dropped directly into an STM32 project: it provides basic, reliable, low-overhead, easy-to-control UART logging without introducing a heavyweight formatting system.

Suitable for:

- STM32 HAL projects
- firmware that does not want to pull in `printf`
- MCUs with limited Flash / RAM
- projects that need quick UART debug logging
- projects that want to switch between blocking logging and DMA logging
- STM32H7 / F7-class targets that need to care about DMA cache and memory placement

Not suitable for:

- projects that need full formatted string support
- projects that need multi-backend logging
- projects that need tags, colors, filesystem logging, or other complex features
- non-STM32 HAL projects

---

## Related Yoru Libraries

Yorulog is part of the Yoru family of lightweight STM32 utility libraries.

These libraries follow similar design principles: single-header where practical, low dependency surface, low resource usage, and a strong focus on STM32 HAL projects. They do not depend on each other by force. Each library can be used independently or combined as needed.

| Library | Role | Description |
| --- | --- | --- |
| [Yorulog](https://github.com/ExMikuPro/Yorulog) | UART logger | This repository. Provides lightweight UART logging with `MINI` / `FULL` modes and optional UART TX DMA |
| [Yorush](https://github.com/ExMikuPro/Yorush) | UART shell / command parser | Provides lightweight serial command interaction, including command dispatch, argument parsing, and help output, with optional Yorulog-based output |
| [Yorunvm](https://github.com/ExMikuPro/Yorunvm) | NVM / Flash / EEPROM helper | Lightweight helper for STM32 on-chip non-volatile storage, mainly wrapping Flash / EEPROM access and region protection |
| [Yorukv](https://github.com/ExMikuPro/Yorukv) | KV config library | Lightweight key-value configuration layer with fixed-table registration, common base-type access, and optional single-region log-style persistence |
| [Yoruassert](https://github.com/ExMikuPro/Yoruassert.git) | Assertion helper | Lightweight assertion helper with hook-based failure handling, intended for embedded runtime checks without forcing one output path |

### Example Combinations

These libraries can be integrated independently or combined into a more complete set of debug, configuration, and on-chip storage building blocks:

- `Yorulog`: only need basic UART logging
- `Yorush`: only need a lightweight serial command entry point
- `Yorunvm`: only need controlled access to STM32 on-chip Flash / EEPROM
- `Yorukv`: only need a simple key-value configuration layer
- `Yoruassert`: only need lightweight runtime assertions and failure hooks
- `Yorulog + Yorush`: view logs, run debug commands, and print help text over UART
- `Yorunvm + Yorukv`: store configuration items in STM32 on-chip Flash / EEPROM
- `Yorulog + Yorukv`: log configuration load, save, reset, and persistence errors
- `Yorush + Yorukv`: inspect, modify, and reset configuration items through serial commands
- `Yorulog + Yoruassert`: print assertion failures through the same lightweight UART log path when needed
- `Yorulog + Yorush + Yorunvm + Yorukv`: build a lightweight combination of logging, command interaction, on-chip storage, and configuration management

The goal of the Yoru family is not to become a large general-purpose framework. The goal is to provide a set of small utility libraries that can be dropped directly into STM32 projects. Each library tries to remain independent, trim, and replaceable.

### Layering

If several libraries are used together, they can be viewed like this:

```text
Yorush   -> serial command entry
Yorulog  -> log output
Yorukv   -> configuration management
Yorunvm  -> STM32 on-chip Flash / EEPROM access helper
Yoruassert -> runtime assertion / failure hook
```

Yorukv can attach to different storage backends through backend callbacks. For STM32 on-chip Flash / EEPROM use cases, Yorunvm can be used as the lower-level NVM access layer.

---

## Why not use `printf`

On small MCUs, `printf` is convenient, but the tradeoff is usually obvious.

It often pulls in extra formatting runtime code, which can lead to:

- higher Flash usage
- higher stack usage
- less predictable link results
- even larger code size when floating-point formatting is enabled
- unnecessary overhead for projects that only need simple logs

Yorulog avoids the format parser and only keeps the output capabilities most commonly needed in embedded debugging. That keeps the implementation simpler and code size easier to control.

---

## Quick Start

### 1. Add the header

Place `yorulog.h` into your project, for example:

```text
Core/Yorulog/yorulog.h
```

Then include it in the files that need logging:

```c
#include "yorulog.h"
```

---

### 2. Define the global implementation in one `.c` file

Yorulog is a single-header library. You need to define `YORULOG_DEFINE_GLOBALS` in exactly one `.c` file.

Placing it in `main.c` or a dedicated `yorulog_port.c` is recommended:

```c
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

All other `.c` files should only include it normally:

```c
#include "yorulog.h"
```

---

### 3. Initialize UART

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

## Select the Operating Mode

Yorulog provides two modes: `MINI` and `FULL`.

| Mode | TX path | RAM usage | DMA | Suitable for |
| --- | --- | ---: | --- | --- |
| `MINI` | blocking UART transmit | lowest | not used | very small projects, a few boot logs, minimal production-safe logging |
| `FULL` | ring buffer + optional DMA | higher | supported | debug phase, heavier logging, lower blocking in application code |

---

### MINI Mode

`MINI` mode always uses blocking UART transmission.

It has no ring buffer and does not use DMA. The advantage is a very small footprint and simple behavior. The downside is that application code waits directly for UART transmission when logs are frequent.

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 1

#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

Suitable for:

- MCUs with very small Flash / RAM
- projects that only need a small amount of boot-time logging
- projects that do not want any extra buffer
- projects that can accept short blocking in the main loop during logging

---

### FULL Mode

`FULL` mode uses a ring buffer.

If the UART handle already has TX DMA configured, Yorulog automatically uses DMA for transmission. If TX DMA is not configured, it automatically falls back to blocking transmission.

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1

#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

Suitable for:

- debug phases with heavier logging
- projects that want to reduce UART line-speed blocking
- projects that already configured UART TX DMA in CubeMX / HAL
- use cases involving shell output, help menus, or longer text output

The default behavior favors log completeness.  
When the buffer becomes full, the default action is to block or flush instead of silently dropping new logs.

---

## FULL Mode and DMA

In `FULL` mode, Yorulog uses UART TX DMA if this condition is true:

```c
huart->hdmatx != NULL
```

You need to forward the HAL UART transmit-complete callback to Yorulog:

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}
```

If TX DMA is not configured, `FULL` mode still works and simply falls back to blocking transmission.

---

## STM32H7 / F7 Notes

STM32H7 has a more special memory layout than many other STM32 series. In many projects, `.data` / `.bss` are placed in DTCM by default, but **DMA cannot access DTCM**.

For that reason, Yorulog places the DMA TX buffer into a DMA-accessible section by default on STM32H7.

Default section name:

```c
#define YORULOG_DMA_SECTION ".RAM_D2"
```

Your linker script needs to place this section into DMA-accessible RAM, for example:

```ld
.RAM_D2 (NOLOAD) :
{
  . = ALIGN(32);
  KEEP(*(.RAM_D2))
  KEEP(*(.RAM_D2*))
  . = ALIGN(32);
} >RAM_D2
```

If D-Cache is enabled, DMA buffer coherency also needs attention.

On H7 / F7-class Cortex-M7 targets, Yorulog cleans the D-Cache by default before starting DMA TX:

```c
#define YORULOG_DMA_CACHE_CLEAN 1
#define YORULOG_DMA_CACHE_LINE_SIZE 32
```

A more robust setup is to use MPU and mark the DMA buffer region as non-cacheable.

---

## API

### Initialization

```c
YORULOG_Init(&huart1);
```

---

### Set Log Level

```c
YORULOG_SetLevel(YORULOG_LEVEL_INFO);
```

Filtering rule: only logs whose severity is not lower than the current level are printed.

For example, after setting `YORULOG_LEVEL_INFO`:

- `ERROR` is printed
- `WARN` is printed
- `INFO` is printed
- `DEBUG` is ignored
- `TRACE` is ignored

---

### Log Output

```c
YORULOG_LogError("fatal");
YORULOG_LogWarn("warn");
YORULOG_LogInfo("info");
YORULOG_LogDebug("debug");
YORULOG_LogTrace("trace");
```

The logging macros take one argument and automatically recognize common types:

| Type | Example |
| --- | --- |
| `const char*` / `char*` | `YORULOG_LogInfo("boot");` |
| `char` | `YORULOG_LogDebug('A');` |
| signed integer | `YORULOG_LogDebug(-123);` |
| unsigned integer | `YORULOG_LogDebug(123u);` |
| pointer | `YORULOG_LogWarn((void*)0x20000000);` |

---

### Tagged Log Output

If you want to keep a small module or feature marker in the log line, you can use the `Tag` macros:

```c
YORULOG_LogTag("boot", "start");
YORULOG_LogInfoTag("net", "link up");
YORULOG_LogWarnTag("sensor", 3);
YORULOG_LogErrorTag("flash", (void*)0x08020000);
```

Typical output looks like:

```text
[boot] start
[I] [net] link up
[W] [sensor] 3
[E] [flash] 0x08020000
```

If `tag` is `NULL` or an empty string, the extra tag part is skipped automatically.

---

### Raw Output

```c
YORULOG_Print("x=");
YORULOG_Print(123);
YORULOG_Println("done");
```

`YORULOG_Print()` does not append a newline.  
`YORULOG_Println()` appends a newline after the output.

`YORULOG_PrintRaw()` and `YORULOG_PrintRawln()` are also provided as explicit raw-output aliases:

```c
YORULOG_PrintRaw("AT+RST");
YORULOG_PrintRawln("\r");
```

They currently follow the same output path as `YORULOG_Print()` / `YORULOG_Println()`, but the naming is useful when you want the call site to clearly express "plain output without log level prefix".

If you want to append the call site to plain line output, you can enable:

```c
#define YORULOG_TRACE_PRINT_CALLSITE 1
```

Example:

```text
done (main.c:123)
```

---

### Long Text Output

If you need to print help menus, shell prompts, or longer descriptive text, the long-text APIs are recommended:

```c
YORULOG_PrintLong("help: ");
YORULOG_PrintLongln("show all commands");
```

In `FULL` mode, the long-text path flushes pending buffered logs first, then uses blocking UART transmission for that long text.  
In `MINI` mode, its behavior is the same as the normal blocking path.

This path is better suited to use cases where complete line output matters more than non-blocking behavior.

---

### Manual Flush

```c
YORULOG_Flush();
```

In `FULL` mode, this can be used to actively flush buffered data.

---

### Optional Yoruassert Integration

Yorulog can optionally use Yoruassert for internal parameter and state checks:

```c
#define YORULOG_USE_YORUASSERT 1
#define YORULOG_YORUASSERT_HEADER "../Yoruassert/yoruassert.h"
```

If you also want invalid logger usage to trigger assertions, keep:

```c
#define YORULOG_ASSERT_ON_ERROR 1
```

Typical examples include calling `YORULOG_Init(NULL)`, printing before init, or passing an invalid log level.

---

### Optional Callsite Tail

If you want complete log APIs to append the caller location, enable:

```c
#define YORULOG_TRACE_LOG_CALLSITE 1
```

Example:

```text
[I] boot ok (main.c:123)
```

---

## Configuration Macros

All configuration macros must be defined before including `yorulog.h`.

| Macro | Default | Description |
| --- | ---: | --- |
| `YORULOG_ENABLE` | `1` | master enable |
| `YORULOG_MINI` | `0` | `1` = MINI mode, `0` = FULL mode |
| `YORULOG_TX_BUF_SIZE` | `512` | ring buffer size in FULL mode |
| `YORULOG_DEFAULT_LEVEL` | `4` | default log level, which means `TRACE` |
| `YORULOG_TIMESTAMP` | `0` | whether to print timestamps |
| `YORULOG_NOW_MS()` | `HAL_GetTick()` | custom millisecond time source |
| `YORULOG_CRLF` | `1` | whether to use `\r\n` line endings |
| `YORULOG_DROP_NEW_ON_FULL` | `0` | whether to drop new data when the buffer is full |
| `YORULOG_BLOCK_ON_FULL` | `1` | whether to block or flush when the buffer is full |
| `YORULOG_FORCE_BLOCKING_EW` | `1` | whether `ERROR` / `WARN` force a flush |
| `YORULOG_USE_YORUASSERT` | `0` | enable optional Yoruassert integration |
| `YORULOG_YORUASSERT_HEADER` | `"../Yoruassert/yoruassert.h"` | custom Yoruassert include path |
| `YORULOG_ASSERT_ON_ERROR` | `1` | assert on invalid logger usage when Yoruassert is enabled |
| `YORULOG_TRACE_PRINT_CALLSITE` | `0` | append `(file:line)` to `Println` / `PrintRawln` |
| `YORULOG_TRACE_LOG_CALLSITE` | `0` | append `(file:line)` to `LogTag` / `LogXxx` / `LogXxxTag` |
| `YORULOG_DMA_SECTION` | `".RAM_D2"` by default on H7 | section used for the DMA buffer |
| `YORULOG_DMA_CACHE_CLEAN` | enabled by default on H7 / F7-class targets | whether to clean D-Cache before DMA TX |
| `YORULOG_DMA_CACHE_LINE_SIZE` | `32` | cache line size used for cache-clean alignment |
| `YORULOG_PREFIX_E/W/I/D/T` | `"[E] "` and so on | custom level prefixes |

Configuration now consistently uses the `YORULOG_*` prefix.  
The old `STLOG_*` configuration macros are no longer kept.

---

## Behavior Notes

### When the Buffer Becomes Full

In `FULL` mode, Yorulog prioritizes log completeness by default.  
When the ring buffer becomes full, the default behavior is to block or flush rather than directly dropping new logs.

If you care more about keeping application code from being blocked by logging, you can adjust it like this:

```c
#define YORULOG_DROP_NEW_ON_FULL 1
#define YORULOG_BLOCK_ON_FULL 0
```

With that configuration, new logs are dropped when the buffer is full.

---

### ERROR / WARN Flush Strategy

By default, `ERROR` and `WARN` force a flush to reduce the chance that critical logs remain buffered:

```c
#define YORULOG_FORCE_BLOCKING_EW 1
```

If you want all log levels to follow the buffered path as much as possible, you can disable it:

```c
#define YORULOG_FORCE_BLOCKING_EW 0
```

---

### Working with Shell / Command Help

If Yorulog is used together with shell-style middleware such as Yorush:

- `MINI` works correctly, but long help text is fully blocking and may pause the main loop briefly.
- `FULL` uses more RAM, but is generally the better balance when logs are heavier or command output is longer.
- For help menus, command lists, and other long text, `YORULOG_PrintLong()` / `YORULOG_PrintLongln()` are recommended.

---

## Size Measurements

Test platform:

- MCU: STM32H743
- Project: current repository test project
- Build preset: Release

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

`MINI` mode does not keep the `FULL`-mode DMA ring buffer, so `RAM_D2` stays at `+0 B` in this measurement.

---

## Performance Measurements

The test platform is also STM32H743.

Test setup:

- USART1
- Baud rate: 115200
- `FULL` mode
- TIM used for `1 us` timing
- DWT used for cycle counting

### Results

| Test item | Data size | Enqueue time | Total time | Throughput |
| --- | ---: | ---: | ---: | ---: |
| `raw println x8` | 272 B | 163 us | 23728 us | 11463 B/s |
| `info log x6` | 228 B | 138 us | 32367 us | 7044 B/s |
| `u32 print x32` | 54 B | 145 us | 16513 us | 3270 B/s |

### Cycle Counts

| Test item | total | per_call | per_byte |
| --- | ---: | ---: | ---: |
| `raw println x8 cycles` | 10437 | 1304 | 38 |
| `info log x6 cycles` | 8790 | 1465 | 38 |
| `u32 print x32 cycles` | 9281 | 290 | 171 |

### Summary

With this setup, Yorulog's enqueue overhead is roughly in the tens of microseconds per call.

The string output path is about `38 cycles/byte`.  
Integer output costs more per byte because it needs number-to-string conversion.

End-to-end latency is mainly limited by the `115200` UART line rate, so the bottleneck is not the logger itself.

---

## Recommended Configurations

### Extreme Low Footprint

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 0
```

Suitable for small Flash / RAM MCUs, or projects that only need a small amount of boot logging.

---

### General Debugging

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1
```

Suitable for most debugging phases.

---

### More Focused on Avoiding Application Blocking

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_DROP_NEW_ON_FULL 1
#define YORULOG_BLOCK_ON_FULL 0
#define YORULOG_FORCE_BLOCKING_EW 0
```

Suitable for scenarios where logging must not interfere with real-time tasks. The tradeoff is that logs may be dropped when the buffer becomes full.

---

## License

This project is released under the MIT License.

See [LICENSE](./LICENSE) for details.
