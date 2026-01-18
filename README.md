# easySTlogger

A tiny, **single-header** UART logger for STM32 (HAL) — designed for **minimum Flash/RAM**, with **log levels**, **prefix**, optional **timestamp**, and an optional **FULL mode** (ring buffer + auto DMA TX).

> No `printf`, no format parser, no heavy stdlib dependency.  
> Just drop in one header and log.

---

## Features

- ✅ **Single header** (`stlog.h`) — drop-in
- ✅ **HAL-only** (UART) — works across STM32 series
- ✅ **Two modes**
  - **MINI**: ultra low Flash/RAM, direct blocking transmit (simplest + smallest)
  - **FULL**: ring buffer + **auto DMA TX** when `huart->hdmatx` exists (non-blocking friendly)
- ✅ **Log levels**: `ERROR/WARN/INFO/DEBUG/TRACE`
- ✅ **Prefixes** per level (`[E] [W] [I] [D] [T]`)
- ✅ **Optional timestamp**: `[HAL_GetTick()]` ms
- ✅ **Type-adaptive** `print()` / `println()` using C11 `_Generic`
  - supports: string, char, signed/unsigned integers, pointer
- ✅ Linker-friendly: `-ffunction-sections -fdata-sections -Wl,--gc-sections` benefits greatly

---

## Why not `printf`?

On small MCUs, `printf` often pulls in large formatting code (and sometimes float support), increasing Flash and stack usage.

This logger avoids format parsing entirely and keeps code size predictable.

---

## Size Comparison (Measured Data)

All numbers below are **real measurements**, compiled with:

- `-O3`
- `--specs=nano.specs`
- `-ffunction-sections -fdata-sections -Wl,--gc-sections`
- Target: **STM32G031 (64 KB Flash / 8 KB RAM)**

### Baseline (HAL + USART1 only)

| Resource | Usage |
|--------|-------|
| RAM    | 1728 B |
| Flash  | 7904 B |

---

### MINI mode enabled

| Resource | Usage | Increment |
|--------|-------|-----------|
| RAM    | 1736 B | **+8 B** |
| Flash  | 8288 B | **+384 B** |

> MINI mode overhead is effectively negligible.  
> It is safe to keep enabled even in production firmware.

---

### FULL mode enabled (ring buffer + auto DMA)

| Resource | Usage | Increment |
|--------|-------|-----------|
| RAM    | 2264 B | **+536 B** |
| Flash  | 9096 B | **+1192 B** |

> RAM increase mainly comes from the ring buffer.  
> Flash increase represents the full cost of buffering, log levels, and DMA handling.

---

### Compared to `printf`

Typical `printf`-based UART logging on STM32:

| Method | Flash Increase |
|------|----------------|
| `printf("%d")` | +3 KB ~ +8 KB |
| `printf("%f")` | +10 KB ~ +20 KB |
| **easySTlogger (MINI)** | **+0.38 KB** |
| **easySTlogger (FULL)** | **+1.2 KB** |

easySTlogger avoids pulling in `vfprintf`, float formatting, and large stack usage.


## Quick Start

### 1) Add `stlog.h` to your project

Put `stlog.h` somewhere like:

```
/Core/Inc/stlog.h
```

Then include it:

```c
#include "stlog.h"
```

### 2) Select mode via macros (before include)

#### MINI mode (ultra small)

```c
#define STLOG_MINI 1
#define STLOG_TIMESTAMP 1   // optional
#include "stlog.h"
```

#### FULL mode (ring + auto DMA)

```c
#define STLOG_MINI 0
#define STLOG_TX_BUF_SIZE 512
#define STLOG_TIMESTAMP 1
#include "stlog.h"
```

### 3) Init with USART handle

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

## FULL mode: DMA TX completion hook (important)

If `huart1.hdmatx != NULL`, FULL mode will try to use DMA automatically.  
You must forward HAL TX complete callback:

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    stlog_on_tx_cplt(huart);
}
```

If you do not use DMA TX, FULL mode falls back to blocking transmit.

---

## API

### Logging macros (with prefix + newline)

```c
loge("fatal");
logw("warn");
logi("info");
logd("debug");
logt("trace");
```

Each takes **one argument** (type-adaptive):

- `const char*` / `char*`
- `char`
- integer types
- pointer

### Raw print without prefix

```c
print("x=");
print(123);
println("done");
```

### Set log level at runtime

```c
log_set_level(LOG_INFO);  // filters out DEBUG/TRACE
```

---

## Configuration Macros

Define these before including `stlog.h`.

| Macro | Default | Meaning |
|---|---:|---|
| `STLOG_ENABLE` | `1` | master on/off |
| `STLOG_MINI` | `0` | `1` = MINI, `0` = FULL |
| `STLOG_TX_BUF_SIZE` | `512` | FULL mode ring size |
| `STLOG_DROP_NEW_ON_FULL` | `1` | FULL: drop new or overwrite old |
| `STLOG_DEFAULT_LEVEL` | `LOG_INFO` | startup log level |
| `STLOG_TIMESTAMP` | `0` | prefix timestamp `[ms]` |
| `STLOG_NOW_MS()` | `HAL_GetTick()` | override time source |
| `STLOG_CRLF` | `1` | `\r\n` newline |
| `STLOG_PREFIX_E/W/I/D/T` | `"[E] "` etc | per-level prefixes |
| `STLOG_FORCE_BLOCKING_EW` | `1` | force ERROR/WARN to flush/block (FULL) |

---

## Notes / Gotchas

### MINI mode behavior
- Uses **blocking UART transmit** for every character/string/number.
- Great for smallest footprint, but heavy logging can stall the CPU (expected).

### FULL mode behavior
- Uses ring buffer and tries DMA automatically when available.
- If you use STM32H7 with D-Cache enabled, DMA buffer location may require MPU non-cacheable or cache maintenance.

---

