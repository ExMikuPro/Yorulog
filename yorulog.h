#ifndef YORULOG_H
#define YORULOG_H

/* =========================================================
 *  Auto-detect and include STM32 HAL header
 * =========================================================
 * Manual override example:
 *   #define YORULOG_HAL_HEADER "stm32h7xx_hal.h"
 *   #include "yorulog.h"
 */
#ifndef YORULOG_HAL_HEADER
  #ifdef STLOG_HAL_HEADER
    #define YORULOG_HAL_HEADER STLOG_HAL_HEADER
  #endif
#endif
#ifndef YORULOG_HAL_HEADER
  #if defined(__has_include)
    #if __has_include("stm32h7xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32h7xx_hal.h"
    #elif __has_include("stm32f7xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32f7xx_hal.h"
    #elif __has_include("stm32f4xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32f4xx_hal.h"
    #elif __has_include("stm32g4xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32g4xx_hal.h"
    #elif __has_include("stm32g0xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32g0xx_hal.h"
    #elif __has_include("stm32l4xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32l4xx_hal.h"
    #elif __has_include("stm32l0xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32l0xx_hal.h"
    #elif __has_include("stm32f1xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32f1xx_hal.h"
    #elif __has_include("stm32c0xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32c0xx_hal.h"
    #elif __has_include("stm32u5xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32u5xx_hal.h"
    #elif __has_include("stm32wbxx_hal.h")
      #define YORULOG_HAL_HEADER "stm32wbxx_hal.h"
    #elif __has_include("stm32wlxx_hal.h")
      #define YORULOG_HAL_HEADER "stm32wlxx_hal.h"
    #elif __has_include("stm32xx_hal.h")
      #define YORULOG_HAL_HEADER "stm32xx_hal.h"
    #endif
  #endif
#endif

#ifdef YORULOG_HAL_HEADER
  #include YORULOG_HAL_HEADER
#else
  #error "yorulog.h: Please define YORULOG_HAL_HEADER before including yorulog.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================
 *  User Configuration
 * ========================================================= */

/* 日志总开关：1=启用 0=禁用 */
#ifndef YORULOG_ENABLE
  #ifdef STLOG_ENABLE
    #define YORULOG_ENABLE STLOG_ENABLE
  #else
    #define YORULOG_ENABLE 1
  #endif
#endif

/* 模式选择：
 * 0 = FULL 模式（环形缓冲区 + DMA 自动发送）
 * 1 = MINI 模式（无缓冲区，直接阻塞发送，超低 Flash/RAM 占用）
 */
#ifndef YORULOG_MINI
  #ifdef STLOG_MINI
    #define YORULOG_MINI STLOG_MINI
  #else
    #define YORULOG_MINI 0
  #endif
#endif

/* 换行模式：1=输出 \r\n（Windows 风格） 0=仅 \n */
#ifndef YORULOG_CRLF
  #ifdef STLOG_CRLF
    #define YORULOG_CRLF STLOG_CRLF
  #else
    #define YORULOG_CRLF 1
  #endif
#endif

/* 时间戳开关：1=在日志前添加 [tick_ms]（使用 HAL_GetTick） */
#ifndef YORULOG_TIMESTAMP
  #ifdef STLOG_TIMESTAMP
    #define YORULOG_TIMESTAMP STLOG_TIMESTAMP
  #else
    #define YORULOG_TIMESTAMP 0
  #endif
#endif

/* 默认日志等级（0~4）：0=ERROR, 1=WARN, 2=INFO, 3=DEBUG, 4=TRACE */
#ifndef YORULOG_DEFAULT_LEVEL
  #ifdef STLOG_DEFAULT_LEVEL
    #define YORULOG_DEFAULT_LEVEL STLOG_DEFAULT_LEVEL
  #else
    #define YORULOG_DEFAULT_LEVEL 4 /* TRACE */
  #endif
#endif

/* 日志前缀自定义 */
#ifndef YORULOG_PREFIX_E
  #ifdef STLOG_PREFIX_E
    #define YORULOG_PREFIX_E STLOG_PREFIX_E
  #else
    #define YORULOG_PREFIX_E "[E] "
  #endif
#endif
#ifndef YORULOG_PREFIX_W
  #ifdef STLOG_PREFIX_W
    #define YORULOG_PREFIX_W STLOG_PREFIX_W
  #else
    #define YORULOG_PREFIX_W "[W] "
  #endif
#endif
#ifndef YORULOG_PREFIX_I
  #ifdef STLOG_PREFIX_I
    #define YORULOG_PREFIX_I STLOG_PREFIX_I
  #else
    #define YORULOG_PREFIX_I "[I] "
  #endif
#endif
#ifndef YORULOG_PREFIX_D
  #ifdef STLOG_PREFIX_D
    #define YORULOG_PREFIX_D STLOG_PREFIX_D
  #else
    #define YORULOG_PREFIX_D "[D] "
  #endif
#endif
#ifndef YORULOG_PREFIX_T
  #ifdef STLOG_PREFIX_T
    #define YORULOG_PREFIX_T STLOG_PREFIX_T
  #else
    #define YORULOG_PREFIX_T "[T] "
  #endif
#endif

/* FULL 模式环形缓冲区大小（字节数，越小越省 RAM） */
#ifndef YORULOG_TX_BUF_SIZE
  #ifdef STLOG_TX_BUF_SIZE
    #define YORULOG_TX_BUF_SIZE STLOG_TX_BUF_SIZE
  #else
    #define YORULOG_TX_BUF_SIZE 512u
  #endif
#endif

/* =========================================================
 *  Platform Feature Detection
 * ========================================================= */

/* Platform family tags */
#if defined(STM32H7)
  #define YORULOG_PLATFORM_STM32H7 1
#else
  #define YORULOG_PLATFORM_STM32H7 0
#endif

#if defined(STM32F7)
  #define YORULOG_PLATFORM_STM32F7 1
#else
  #define YORULOG_PLATFORM_STM32F7 0
#endif

/* Capability tags
 * - H7 默认把 .data/.bss 放在 DTCM 的工程很常见，DMA 不能访问；
 *   因此为 H7 自动开启 DMA 可访问缓冲区支持。
 * - 其他系列默认关闭，保持最小接入面；如有需要可手动 override。
 */
#ifndef YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER
  #if YORULOG_PLATFORM_STM32H7
    #define YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER 1
  #else
    #define YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER 0
  #endif
#endif

/* H7/F7 这类可能带 D-Cache 的平台可在后续需要时用它扩展 cache 维护策略。 */
#ifndef YORULOG_HAS_DCACHE_RISK
  #if YORULOG_PLATFORM_STM32H7 || YORULOG_PLATFORM_STM32F7
    #define YORULOG_HAS_DCACHE_RISK 1
  #else
    #define YORULOG_HAS_DCACHE_RISK 0
  #endif
#endif

/* FULL 模式 DMA 发送缓冲区所在 section
 * STM32H7 上默认 .bss/.data 常在 DTCM，DMA 无法访问；
 * 因此默认把发送缓冲区单独放到可 DMA 访问的 RAM_D2 section。
 */
#ifndef YORULOG_DMA_SECTION
  #ifdef STLOG_DMA_SECTION
    #define YORULOG_DMA_SECTION STLOG_DMA_SECTION
  #else
    #define YORULOG_DMA_SECTION ".RAM_D2"
  #endif
#endif

#if defined(__APPLE__)
#define YORULOG_SECTION_ATTR(name)
#elif YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER
#define YORULOG_SECTION_ATTR(name) __attribute__((section(name)))
#else
#define YORULOG_SECTION_ATTR(name)
#endif

/* 缓冲区满时的处理策略：
 * 1 = 丢弃新数据（不阻塞，最稳定）
 * 0 = 覆盖旧数据（更实时）
 */
#ifndef YORULOG_DROP_NEW_ON_FULL
  #ifdef STLOG_DROP_NEW_ON_FULL
    #define YORULOG_DROP_NEW_ON_FULL STLOG_DROP_NEW_ON_FULL
  #else
    #define YORULOG_DROP_NEW_ON_FULL 0
  #endif
#endif

/* 缓冲区满时阻塞等待腾出空间：
 * 1 = 优先保证日志完整，不额外增加 RAM，占用高峰时会阻塞
 * 0 = 维持旧行为（配合 YORULOG_DROP_NEW_ON_FULL 决定丢新/覆盖旧）
 *
 * 对于 STM32H7 这类 DMA logger，小缓冲区场景下推荐开启。
 */
#ifndef YORULOG_BLOCK_ON_FULL
  #ifdef STLOG_BLOCK_ON_FULL
    #define YORULOG_BLOCK_ON_FULL STLOG_BLOCK_ON_FULL
  #else
    #define YORULOG_BLOCK_ON_FULL 1
  #endif
#endif

/* ERROR/WARN 日志强制阻塞发送（推荐开启）：
 * 1 = ERROR/WARN 会阻塞等待发送完成，确保重要日志不丢失
 * 0 = 所有日志使用相同的发送策略
 * - FULL 模式：ERROR/WARN 会刷新缓冲区（必要时阻塞）
 * - MINI 模式：本身就是阻塞发送
 */
#ifndef YORULOG_FORCE_BLOCKING_EW
  #ifdef STLOG_FORCE_BLOCKING_EW
    #define YORULOG_FORCE_BLOCKING_EW STLOG_FORCE_BLOCKING_EW
  #else
    #define YORULOG_FORCE_BLOCKING_EW 1
  #endif
#endif

/* 并发保护锁（防止中断/多线程输出混乱）
 * 自定义示例：
 *   #define YORULOG_LOCK()   uint32_t pm=__get_PRIMASK(); __disable_irq();
 *   #define YORULOG_UNLOCK() if(!pm) __enable_irq();
 */
#ifndef YORULOG_LOCK
  #ifdef STLOG_LOCK
    #define YORULOG_LOCK() STLOG_LOCK()
  #else
    #define YORULOG_LOCK()   do{}while(0)
  #endif
#endif
#ifndef YORULOG_UNLOCK
  #ifdef STLOG_UNLOCK
    #define YORULOG_UNLOCK() STLOG_UNLOCK()
  #else
    #define YORULOG_UNLOCK() do{}while(0)
  #endif
#endif

/* 时间戳来源（可自定义替换 HAL_GetTick） */
#ifndef YORULOG_NOW_MS
  #ifdef STLOG_NOW_MS
    #define YORULOG_NOW_MS() STLOG_NOW_MS()
  #else
    #define YORULOG_NOW_MS() ((unsigned)HAL_GetTick())
  #endif
#endif

/* Backward-compatible config aliases */
#ifndef STLOG_ENABLE
#define STLOG_ENABLE YORULOG_ENABLE
#endif
#ifndef STLOG_MINI
#define STLOG_MINI YORULOG_MINI
#endif
#ifndef STLOG_CRLF
#define STLOG_CRLF YORULOG_CRLF
#endif
#ifndef STLOG_TIMESTAMP
#define STLOG_TIMESTAMP YORULOG_TIMESTAMP
#endif
#ifndef STLOG_DEFAULT_LEVEL
#define STLOG_DEFAULT_LEVEL YORULOG_DEFAULT_LEVEL
#endif
#ifndef STLOG_PREFIX_E
#define STLOG_PREFIX_E YORULOG_PREFIX_E
#endif
#ifndef STLOG_PREFIX_W
#define STLOG_PREFIX_W YORULOG_PREFIX_W
#endif
#ifndef STLOG_PREFIX_I
#define STLOG_PREFIX_I YORULOG_PREFIX_I
#endif
#ifndef STLOG_PREFIX_D
#define STLOG_PREFIX_D YORULOG_PREFIX_D
#endif
#ifndef STLOG_PREFIX_T
#define STLOG_PREFIX_T YORULOG_PREFIX_T
#endif
#ifndef STLOG_TX_BUF_SIZE
#define STLOG_TX_BUF_SIZE YORULOG_TX_BUF_SIZE
#endif
#ifndef STLOG_NEEDS_DMA_ACCESSIBLE_BUFFER
#define STLOG_NEEDS_DMA_ACCESSIBLE_BUFFER YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER
#endif
#ifndef STLOG_HAS_DCACHE_RISK
#define STLOG_HAS_DCACHE_RISK YORULOG_HAS_DCACHE_RISK
#endif
#ifndef STLOG_DMA_SECTION
#define STLOG_DMA_SECTION YORULOG_DMA_SECTION
#endif
#ifndef STLOG_DROP_NEW_ON_FULL
#define STLOG_DROP_NEW_ON_FULL YORULOG_DROP_NEW_ON_FULL
#endif
#ifndef STLOG_BLOCK_ON_FULL
#define STLOG_BLOCK_ON_FULL YORULOG_BLOCK_ON_FULL
#endif
#ifndef STLOG_FORCE_BLOCKING_EW
#define STLOG_FORCE_BLOCKING_EW YORULOG_FORCE_BLOCKING_EW
#endif
#ifndef STLOG_LOCK
#define STLOG_LOCK() YORULOG_LOCK()
#endif
#ifndef STLOG_UNLOCK
#define STLOG_UNLOCK() YORULOG_UNLOCK()
#endif
#ifndef STLOG_NOW_MS
#define STLOG_NOW_MS() YORULOG_NOW_MS()
#endif

/* =========================================================
 *  Type Definitions
 * ========================================================= */
typedef enum {
    YORULOG_LEVEL_ERROR = 0,
    YORULOG_LEVEL_WARN  = 1,
    YORULOG_LEVEL_INFO  = 2,
    YORULOG_LEVEL_DEBUG = 3,
    YORULOG_LEVEL_TRACE = 4,
} YORULOG_LevelTypeDef;

typedef struct {
    UART_HandleTypeDef *huart;
    volatile YORULOG_LevelTypeDef level;

#if !STLOG_MINI
    volatile unsigned head;
    volatile unsigned tail;
    volatile unsigned tx_busy;
    volatile unsigned dma_tail;
    volatile unsigned dma_len;
    unsigned char *tx_buf;
#endif
} YORULOG_HandleTypeDef;

typedef YORULOG_LevelTypeDef log_level_t;
typedef YORULOG_HandleTypeDef stlog_t;

#define LOG_ERROR YORULOG_LEVEL_ERROR
#define LOG_WARN  YORULOG_LEVEL_WARN
#define LOG_INFO  YORULOG_LEVEL_INFO
#define LOG_DEBUG YORULOG_LEVEL_DEBUG
#define LOG_TRACE YORULOG_LEVEL_TRACE

/* Global singleton storage.
 * Define YORULOG_DEFINE_GLOBALS in exactly one .c file before including yorulog.h.
 */
#if defined(YORULOG_DEFINE_GLOBALS) || defined(STLOG_DEFINE_GLOBALS)
YORULOG_HandleTypeDef hYorulog;
YORULOG_SECTION_ATTR(YORULOG_DMA_SECTION) __attribute__((aligned(32)))
unsigned char yorulog_tx_buf[YORULOG_TX_BUF_SIZE];
#else
extern YORULOG_HandleTypeDef hYorulog;
extern unsigned char yorulog_tx_buf[YORULOG_TX_BUF_SIZE];
#endif

#define g_stlog hYorulog
#define g_stlog_tx_buf yorulog_tx_buf

/* =========================================================
 *  Internal Helper Functions (no stdlib dependency)
 * ========================================================= */

static inline void YORULOG_Init(UART_HandleTypeDef *huart)
{
    g_stlog.huart = huart;
    g_stlog.level = (YORULOG_LevelTypeDef)STLOG_DEFAULT_LEVEL;

#if !STLOG_MINI
    g_stlog.head = 0u;
    g_stlog.tail = 0u;
    g_stlog.tx_busy = 0u;
    g_stlog.dma_tail = 0u;
    g_stlog.dma_len = 0u;
    g_stlog.tx_buf = g_stlog_tx_buf;
#endif
}

static inline void YORULOG_SetLevel(YORULOG_LevelTypeDef lv)
{
    g_stlog.level = lv;
}

static inline void stlog_init(UART_HandleTypeDef *huart)
{
    YORULOG_Init(huart);
}

static inline void log_set_level(YORULOG_LevelTypeDef lv)
{
    YORULOG_SetLevel(lv);
}

static inline unsigned stlog__lvl_enabled_(YORULOG_LevelTypeDef lv)
{
    return (unsigned)(g_stlog.level >= lv);
}

/* ---------- MINI Mode: Direct blocking transmission ---------- */
#if STLOG_MINI

static inline void stlog__putc_(char c)
{
#if STLOG_ENABLE
    (void)HAL_UART_Transmit(g_stlog.huart, (uint8_t*)&c, 1u, 0xFFFFu);
#else
    (void)c;
#endif
}

static inline void stlog_write_cstr_(const char *s)
{
#if STLOG_ENABLE
    if (!s) return;
    while (*s) stlog__putc_(*s++);
#else
    (void)s;
#endif
}

static inline void stlog_nl_(void)
{
#if STLOG_ENABLE
#if STLOG_CRLF
    stlog__putc_('\r');
#endif
    stlog__putc_('\n');
#endif
}

static inline void stlog_char_(char c)
{
#if STLOG_ENABLE
    stlog__putc_(c);
#else
    (void)c;
#endif
}

static inline void stlog_u32_(unsigned v)
{
#if STLOG_ENABLE
    char buf[10];
    unsigned i = 0u;

    if (v == 0u) { stlog__putc_('0'); return; }

    while (v && i < (unsigned)sizeof(buf)) {
        buf[i++] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    while (i--) stlog__putc_(buf[i]);
#else
    (void)v;
#endif
}

static inline void stlog_u64_(unsigned long long v)
{
#if STLOG_ENABLE
    char buf[20];
    unsigned i = 0u;

    if (v == 0ull) { stlog__putc_('0'); return; }

    while (v && i < (unsigned)sizeof(buf)) {
        buf[i++] = (char)('0' + (char)(v % 10ull));
        v /= 10ull;
    }
    while (i--) stlog__putc_(buf[i]);
#else
    (void)v;
#endif
}

static inline void stlog_i32_(int v)
{
#if STLOG_ENABLE
    if (v < 0) {
        stlog__putc_('-');
        unsigned uv = (unsigned)(0u - (unsigned)v); /* Safe for INT_MIN */
        stlog_u32_(uv);
    } else {
        stlog_u32_((unsigned)v);
    }
#else
    (void)v;
#endif
}

static inline void stlog_i64_(long long v)
{
#if STLOG_ENABLE
    if (v < 0ll) {
        stlog__putc_('-');
        stlog_u64_(0ull - (unsigned long long)v);
    } else {
        stlog_u64_((unsigned long long)v);
    }
#else
    (void)v;
#endif
}

static inline void stlog_hex_u32_(unsigned v)
{
#if STLOG_ENABLE
    for (int i = 7; i >= 0; --i) {
        unsigned nib = (v >> (unsigned)(i * 4)) & 0xFu;
        char c = (char)((nib < 10u) ? ('0' + nib) : ('A' + (nib - 10u)));
        stlog__putc_(c);
    }
#else
    (void)v;
#endif
}

static inline void stlog_ptr_(const void *p)
{
#if STLOG_ENABLE
    stlog_write_cstr_("0x");
    for (int i = (int)(sizeof(void*) * 2u) - 1; i >= 0; --i) {
        unsigned nib = (unsigned)(((unsigned long long)(unsigned long)p >> (unsigned)(i * 4)) & 0xFull);
        char c = (char)((nib < 10u) ? ('0' + nib) : ('A' + (nib - 10u)));
        stlog__putc_(c);
    }
#else
    (void)p;
#endif
}

/* MINI mode doesn't need DMA callback */
static inline void YORULOG_TxCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
}

static inline void stlog_on_tx_cplt(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}

#else /* ---------- FULL Mode: Ring buffer + DMA auto-transmission ---------- */

/* Ring buffer helper functions */
static inline unsigned stlog__next_(unsigned x)
{
    x++;
    if (x >= (unsigned)STLOG_TX_BUF_SIZE) x = 0u;
    return x;
}

static inline unsigned stlog__advance_(unsigned x, unsigned n)
{
    while (n--) x = stlog__next_(x);
    return x;
}

static inline unsigned stlog__seg_len_(const YORULOG_HandleTypeDef *l)
{
    unsigned t = l->tail, h = l->head;
    return (h > t) ? (h - t) : ((unsigned)STLOG_TX_BUF_SIZE - t);
}

static inline void stlog__flush_blocking_(YORULOG_HandleTypeDef *l);

static inline void stlog__push_(YORULOG_HandleTypeDef *l, unsigned char c)
{
    unsigned next = stlog__next_(l->head);

    if (next == l->tail) {
#if STLOG_BLOCK_ON_FULL
        stlog__flush_blocking_(l);
        next = stlog__next_(l->head);
        if (next == l->tail) {
#if STLOG_DROP_NEW_ON_FULL
            return;
#else
            l->tail = stlog__next_(l->tail);
#endif
        }
#elif STLOG_DROP_NEW_ON_FULL
        return;
#else
        if (l->tx_busy) return;
        l->tail = stlog__next_(l->tail);
#endif
    }

    l->tx_buf[l->head] = c;
    l->head = next;
}

static inline void stlog__flush_blocking_(YORULOG_HandleTypeDef *l)
{
#if STLOG_ENABLE
    if (!l || !l->huart) return;
    while (l->tx_busy) {}

    while (l->head != l->tail) {
        unsigned t = l->tail;
        unsigned len = stlog__seg_len_(l);
        if (len == 0u) break;

        if (HAL_UART_Transmit(l->huart, (uint8_t*)&l->tx_buf[t], (uint16_t)len, 0xFFFFu) != HAL_OK) {
            break;
        }

        l->tail = stlog__advance_(t, len);
    }
#else
    (void)l;
#endif
}

/* Kick transmission: auto DMA if available, else blocking flush
 * Key: advance tail before DMA start to prevent duplicate transmission
 */
static inline void stlog__kick_(YORULOG_HandleTypeDef *l)
{
#if STLOG_ENABLE
    if (!l || !l->huart) return;
    if (l->head == l->tail) return;
    if (l->tx_busy) return;

    UART_HandleTypeDef *hu = l->huart;

    if (hu->hdmatx != (DMA_HandleTypeDef*)0) {
        unsigned t = l->tail;
        unsigned len = stlog__seg_len_(l);
        if (len == 0u) return;

        l->dma_tail = t;
        l->dma_len = len;
        l->tx_busy = 1u;
        if (HAL_UART_Transmit_DMA(hu, (uint8_t*)&l->tx_buf[t], (uint16_t)len) != HAL_OK) {
            l->tx_busy = 0u;
            l->dma_len = 0u;
            stlog__flush_blocking_(l);
        }
    } else {
        stlog__flush_blocking_(l);
    }
#else
    (void)l;
#endif
}

/* Must be called from HAL_UART_TxCpltCallback */
static inline void YORULOG_TxCpltCallback(UART_HandleTypeDef *huart)
{
#if STLOG_ENABLE
    if (g_stlog.huart == huart) {
        if (g_stlog.tx_busy) {
            g_stlog.tail = stlog__advance_(g_stlog.dma_tail, g_stlog.dma_len);
            g_stlog.dma_len = 0u;
        }
        g_stlog.tx_busy = 0u;
        stlog__kick_(&g_stlog);
    }
#else
    (void)huart;
#endif
}

static inline void stlog_on_tx_cplt(UART_HandleTypeDef *huart)
{
    YORULOG_TxCpltCallback(huart);
}

/* Output primitives: push to ring buffer then kick transmission */
static inline void stlog_write_cstr_(const char *s)
{
#if STLOG_ENABLE
    if (!s) return;
    while (*s) stlog__push_(&g_stlog, (unsigned char)*s++);
    stlog__kick_(&g_stlog);
#else
    (void)s;
#endif
}

static inline void stlog_nl_(void)
{
#if STLOG_ENABLE
#if STLOG_CRLF
    stlog__push_(&g_stlog, '\r');
#endif
    stlog__push_(&g_stlog, '\n');
    stlog__kick_(&g_stlog);
#endif
}

static inline void stlog_char_(char c)
{
#if STLOG_ENABLE
    stlog__push_(&g_stlog, (unsigned char)c);
    stlog__kick_(&g_stlog);
#else
    (void)c;
#endif
}

static inline void stlog_u32_(unsigned v)
{
#if STLOG_ENABLE
    char buf[10];
    unsigned i = 0u;

    if (v == 0u) {
        stlog__push_(&g_stlog, '0');
        stlog__kick_(&g_stlog);
        return;
    }

    while (v && i < (unsigned)sizeof(buf)) {
        buf[i++] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    while (i--) stlog__push_(&g_stlog, (unsigned char)buf[i]);

    stlog__kick_(&g_stlog);
#else
    (void)v;
#endif
}

static inline void stlog_u64_(unsigned long long v)
{
#if STLOG_ENABLE
    char buf[20];
    unsigned i = 0u;

    if (v == 0ull) {
        stlog__push_(&g_stlog, '0');
        stlog__kick_(&g_stlog);
        return;
    }

    while (v && i < (unsigned)sizeof(buf)) {
        buf[i++] = (char)('0' + (char)(v % 10ull));
        v /= 10ull;
    }
    while (i--) stlog__push_(&g_stlog, (unsigned char)buf[i]);

    stlog__kick_(&g_stlog);
#else
    (void)v;
#endif
}

static inline void stlog_i32_(int v)
{
#if STLOG_ENABLE
    if (v < 0) {
        stlog__push_(&g_stlog, '-');
        unsigned uv = (unsigned)(0u - (unsigned)v);
        stlog_u32_(uv);
    } else {
        stlog_u32_((unsigned)v);
    }
#else
    (void)v;
#endif
}

static inline void stlog_i64_(long long v)
{
#if STLOG_ENABLE
    if (v < 0ll) {
        stlog__push_(&g_stlog, '-');
        stlog_u64_(0ull - (unsigned long long)v);
    } else {
        stlog_u64_((unsigned long long)v);
    }
#else
    (void)v;
#endif
}

static inline void stlog_hex_u32_(unsigned v)
{
#if STLOG_ENABLE
    for (int i = 7; i >= 0; --i) {
        unsigned nib = (v >> (unsigned)(i * 4)) & 0xFu;
        unsigned char c = (unsigned char)((nib < 10u) ? ('0' + nib) : ('A' + (nib - 10u)));
        stlog__push_(&g_stlog, c);
    }
    stlog__kick_(&g_stlog);
#else
    (void)v;
#endif
}

static inline void stlog_ptr_(const void *p)
{
#if STLOG_ENABLE
    stlog_write_cstr_("0x");
    for (int i = (int)(sizeof(void*) * 2u) - 1; i >= 0; --i) {
        unsigned nib = (unsigned)(((unsigned long long)(unsigned long)p >> (unsigned)(i * 4)) & 0xFull);
        unsigned char c = (unsigned char)((nib < 10u) ? ('0' + nib) : ('A' + (nib - 10u)));
        stlog__push_(&g_stlog, c);
    }
    stlog__kick_(&g_stlog);
#else
    (void)p;
#endif
}

#endif /* STLOG_MINI */

/* =========================================================
 *  Prefix and Timestamp Handling
 * ========================================================= */
static inline void stlog__emit_prefix_(YORULOG_LevelTypeDef lv)
{
#if STLOG_ENABLE
#if STLOG_TIMESTAMP
    stlog_write_cstr_("[");
    stlog_u32_(STLOG_NOW_MS());
    stlog_write_cstr_("] ");
#endif

    if (lv == LOG_ERROR) stlog_write_cstr_(STLOG_PREFIX_E);
    else if (lv == LOG_WARN)  stlog_write_cstr_(STLOG_PREFIX_W);
    else if (lv == LOG_INFO)  stlog_write_cstr_(STLOG_PREFIX_I);
    else if (lv == LOG_DEBUG) stlog_write_cstr_(STLOG_PREFIX_D);
    else stlog_write_cstr_(STLOG_PREFIX_T);
#else
    (void)lv;
#endif
}

/* FULL mode: ERROR/WARN can force flush queue; MINI mode is always blocking */
static inline void stlog__commit_level_(YORULOG_LevelTypeDef lv)
{
#if STLOG_ENABLE
#if !STLOG_MINI
#if STLOG_FORCE_BLOCKING_EW
    if (lv <= LOG_WARN) {
        /* Force blocking flush to ensure critical messages are delivered */
        stlog__flush_blocking_(&g_stlog);
        return;
    }
#endif
#endif
    (void)lv;
#else
    (void)lv;
#endif
}

static inline void YORULOG_Flush(void)
{
#if STLOG_ENABLE
#if STLOG_MINI
    return;
#else
    stlog__flush_blocking_(&g_stlog);
#endif
#endif
}

static inline void stlog_flush(void)
{
    YORULOG_Flush();
}

/* =========================================================
 *  Type-adaptive Print Functions (C11 _Generic)
 * ========================================================= */
#define stlog__print_any_(x) \
    _Generic((x), \
        const char*: stlog_write_cstr_, \
        char*:       stlog_write_cstr_, \
        char:        stlog_char_, \
        signed char: stlog_i32_, \
        unsigned char: stlog_u32_, \
        short:       stlog_i32_, \
        unsigned short: stlog_u32_, \
        int:         stlog_i32_, \
        unsigned:    stlog_u32_, \
        long:        stlog_i64_, \
        unsigned long: stlog_u64_, \
        long long:   stlog_i64_, \
        unsigned long long: stlog_u64_, \
        const void*: stlog_ptr_, \
        void*:       stlog_ptr_, \
        default:     stlog_ptr_ \
    )(x)

#if STLOG_ENABLE
  #define YORULOG_Print(x)   do{ YORULOG_LOCK(); stlog__print_any_(x); YORULOG_UNLOCK(); }while(0)
  #define YORULOG_Println(x) do{ YORULOG_LOCK(); stlog__print_any_(x); stlog_nl_(); YORULOG_UNLOCK(); }while(0)
  #define print(x)   YORULOG_Print(x)
  #define println(x) YORULOG_Println(x)
#else
  #define YORULOG_Print(x)   do{ (void)(x); }while(0)
  #define YORULOG_Println(x) do{ (void)(x); }while(0)
  #define print(x)   do{ (void)(x); }while(0)
  #define println(x) do{ (void)(x); }while(0)
#endif

/* =========================================================
 *  Level-based Logging Macros with Prefixes
 * ========================================================= */
#if STLOG_ENABLE
  #define YORULOG_LogError(x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_ERROR)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_ERROR); stlog__print_any_(x); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_ERROR); } }while(0)
  #define YORULOG_LogWarn(x)  do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_WARN )){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_WARN ); stlog__print_any_(x); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_WARN ); } }while(0)
  #define YORULOG_LogInfo(x)  do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_INFO )){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_INFO ); stlog__print_any_(x); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_INFO ); } }while(0)
  #define YORULOG_LogDebug(x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_DEBUG)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_DEBUG); stlog__print_any_(x); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_DEBUG); } }while(0)
  #define YORULOG_LogTrace(x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_TRACE)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_TRACE); stlog__print_any_(x); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_TRACE); } }while(0)
  #define loge(x) YORULOG_LogError(x)
  #define logw(x) YORULOG_LogWarn(x)
  #define logi(x) YORULOG_LogInfo(x)
  #define logd(x) YORULOG_LogDebug(x)
  #define logt(x) YORULOG_LogTrace(x)
#else
  #define YORULOG_LogError(x) do{ (void)(x); }while(0)
  #define YORULOG_LogWarn(x)  do{ (void)(x); }while(0)
  #define YORULOG_LogInfo(x)  do{ (void)(x); }while(0)
  #define YORULOG_LogDebug(x) do{ (void)(x); }while(0)
  #define YORULOG_LogTrace(x) do{ (void)(x); }while(0)
  #define loge(x) do{ (void)(x); }while(0)
  #define logw(x) do{ (void)(x); }while(0)
  #define logi(x) do{ (void)(x); }while(0)
  #define logd(x) do{ (void)(x); }while(0)
  #define logt(x) do{ (void)(x); }while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* YORULOG_H */
