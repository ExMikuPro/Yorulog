# Yorulog

A tiny, **single-header** UART logger for STM32 HAL, designed for **minimum Flash/RAM** while still offering **log levels**, **prefixes**, optional **timestamps**, and an optional **FULL mode** (ring buffer + auto DMA TX).

> No `printf`, no format parser, no heavy stdlib dependency.  
> Just drop in one header and log.

---

## Features

- **Single header**: `yorulog.h`
- **HAL-only**: works across STM32 series using UART
- **Two modes**
  - **MINI**: ultra-small, direct blocking transmit
  - **FULL**: ring buffer + auto DMA TX when `huart->hdmatx` exists
- **Log levels**: `ERROR / WARN / INFO / DEBUG / TRACE`
- **Per-level prefixes**: `[E] [W] [I] [D] [T]`
- **Optional timestamp**: `[HAL_GetTick()]`
- **Type-adaptive** `YORULOG_Print()` / `YORULOG_Println()` using C11 `_Generic`
- **Platform-aware behavior**
  - auto-detects STM32 HAL headers
  - auto-enables DMA-accessible TX buffer support on STM32H7 by default

---

## Why not `printf`?

On small MCUs, `printf` often pulls in a large formatting runtime, increases stack usage, and makes code size less predictable.

Yorulog avoids format parsing entirely, so both code size and runtime behavior stay simple and controllable.

---

## Quick Start

### 1. Add `yorulog.h`

Put the header somewhere in your project, for example:

```text
/Core/easy-ST-Logger/yorulog.h
```

### 2. Define configuration before include

In exactly **one** `.c` file, define `YORULOG_DEFINE_GLOBALS` before including `yorulog.h`.

```c
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

In all other files, just include it normally:

```c
#include "yorulog.h"
```

### 3. Select mode

#### MINI mode

```c
#define YORULOG_MINI 1
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

#### FULL mode

```c
#define YORULOG_MINI 0
#define YORULOG_TX_BUF_SIZE 512
#define YORULOG_TIMESTAMP 1
#define YORULOG_DEFINE_GLOBALS
#include "yorulog.h"
```

### 4. Initialize with a UART handle

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

## FULL Mode and DMA

If `huart->hdmatx != NULL`, FULL mode automatically uses DMA TX.

You must forward the HAL TX complete callback:

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}
```

If no TX DMA is configured, FULL mode falls back to blocking transmit.

---

## STM32H7 Notes

On STM32H7, many projects place default `.data/.bss` in DTCM, which DMA cannot access.  
Yorulog therefore enables a **DMA-accessible TX buffer** automatically on H7 by default.

Default H7 buffer section:

```c
#define YORULOG_DMA_SECTION ".RAM_D2"
```

Your linker script should map that section into DMA-visible RAM, for example:

```ld
.RAM_D2 (NOLOAD) :
{
  . = ALIGN(32);
  KEEP(*(.RAM_D2))
  KEEP(*(.RAM_D2*))
  . = ALIGN(32);
} >RAM_D2
```

If your H7 project enables D-Cache, DMA buffer coherency may still require MPU non-cacheable memory or explicit cache maintenance.

---

## API

### Logging macros

```c
YORULOG_LogError("fatal");
YORULOG_LogWarn("warn");
YORULOG_LogInfo("info");
YORULOG_LogDebug("debug");
YORULOG_LogTrace("trace");
```

Each macro accepts one type-adaptive argument:

- `const char*` / `char*`
- `char`
- signed / unsigned integers
- pointer

### Raw output

```c
YORULOG_Print("x=");
YORULOG_Print(123);
YORULOG_Println("done");
```

### Runtime log level

```c
YORULOG_SetLevel(YORULOG_LEVEL_INFO);
```

### Manual flush

```c
YORULOG_Flush();
```

---

## Configuration Macros

Define these before including `yorulog.h`.

| Macro | Default | Meaning |
|---|---:|---|
| `YORULOG_ENABLE` | `1` | master on/off |
| `YORULOG_MINI` | `0` | `1` = MINI, `0` = FULL |
| `YORULOG_TX_BUF_SIZE` | `512` | FULL mode ring size |
| `YORULOG_DEFAULT_LEVEL` | `4` | startup level = `TRACE` |
| `YORULOG_TIMESTAMP` | `0` | prefix timestamp `[ms]` |
| `YORULOG_NOW_MS()` | `HAL_GetTick()` | override time source |
| `YORULOG_CRLF` | `1` | use `\r\n` |
| `YORULOG_DROP_NEW_ON_FULL` | `0` | FULL: when not blocking, overwrite old instead of dropping new |
| `YORULOG_BLOCK_ON_FULL` | `1` | FULL: block/flush when the ring is full |
| `YORULOG_FORCE_BLOCKING_EW` | `1` | force `ERROR/WARN` to flush/block |
| `YORULOG_DMA_SECTION` | `".RAM_D2"` on H7 | DMA buffer section name |
| `YORULOG_PREFIX_E/W/I/D/T` | `"[E] "` etc | per-level prefixes |

Backward-compatible `STLOG_*` macros and `stlog_*` / `log*` APIs are still available in this release.

---

## Behavior Notes

### MINI mode

- Always uses blocking UART transmit
- Smallest and simplest option
- Best for production firmware with very tight resources

### FULL mode

- Uses a ring buffer
- Automatically switches to DMA TX when available
- Default behavior prioritizes **log completeness over peak non-blocking throughput**
- With the current defaults, a full buffer will block/flush instead of silently truncating output

---

## Measured Size Impact

Measured on **STM32H743** using this repository's current test project and **Release** preset:

- `-Os -g0`
- `--specs=nano.specs`
- `-ffunction-sections -fdata-sections -Wl,--gc-sections`

### Baseline (HAL + USART1 only)

| Resource | Usage |
|---|---:|
| Flash | 15536 B |
| DTCMRAM | 1856 B |
| RAM_D2 | 0 B |
| `arm-none-eabi-size` dec | 17376 |

### MINI mode (library integrated, `YORULOG_Init()` only)

| Resource | Usage | Increment |
|---|---:|---:|
| Flash | 15560 B | +24 B |
| DTCMRAM | 1864 B | +8 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 17920 | +544 |

### FULL mode (library integrated, DMA callback wired, no self-test output)

| Resource | Usage | Increment |
|---|---:|---:|
| Flash | 17092 B | +1556 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 19476 | +2100 |

### FULL mode + bundled self-test in `main.c`

| Resource | Usage | Increment vs baseline |
|---|---:|---:|
| Flash | 18532 B | +2996 B |
| DTCMRAM | 1888 B | +32 B |
| RAM_D2 | 512 B | +512 B |
| `arm-none-eabi-size` dec | 20916 | +3540 |

Notes:

- The self-test strings and demo logic add about `1440 B` of Flash on top of the plain FULL-mode integration.
- In the current implementation, `YORULOG_DEFINE_GLOBALS` still reserves the global `yorulog_tx_buf`, so MINI mode also shows the `512 B` `RAM_D2` allocation.
