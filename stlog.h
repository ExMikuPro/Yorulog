#ifndef STLOG_H
#define STLOG_H

/* =========================================================
 *  Auto-detect and include STM32 HAL header
 * =========================================================
 * Manual override example:
 *   #define STLOG_HAL_HEADER "stm32h7xx_hal.h"
 *   #include "stlog.h"
 */
#ifndef STLOG_HAL_HEADER
  #if defined(__has_include)
    #if __has_include("stm32h7xx_hal.h")
      #define STLOG_HAL_HEADER "stm32h7xx_hal.h"
    #elif __has_include("stm32f7xx_hal.h")
      #define STLOG_HAL_HEADER "stm32f7xx_hal.h"
    #elif __has_include("stm32f4xx_hal.h")
      #define STLOG_HAL_HEADER "stm32f4xx_hal.h"
    #elif __has_include("stm32g4xx_hal.h")
      #define STLOG_HAL_HEADER "stm32g4xx_hal.h"
    #elif __has_include("stm32g0xx_hal.h")
      #define STLOG_HAL_HEADER "stm32g0xx_hal.h"
    #elif __has_include("stm32l4xx_hal.h")
      #define STLOG_HAL_HEADER "stm32l4xx_hal.h"
    #elif __has_include("stm32l0xx_hal.h")
      #define STLOG_HAL_HEADER "stm32l0xx_hal.h"
    #elif __has_include("stm32f1xx_hal.h")
      #define STLOG_HAL_HEADER "stm32f1xx_hal.h"
    #elif __has_include("stm32c0xx_hal.h")
      #define STLOG_HAL_HEADER "stm32c0xx_hal.h"
    #elif __has_include("stm32u5xx_hal.h")
      #define STLOG_HAL_HEADER "stm32u5xx_hal.h"
    #elif __has_include("stm32wbxx_hal.h")
      #define STLOG_HAL_HEADER "stm32wbxx_hal.h"
    #elif __has_include("stm32wlxx_hal.h")
      #define STLOG_HAL_HEADER "stm32wlxx_hal.h"
    #elif __has_include("stm32xx_hal.h")
      #define STLOG_HAL_HEADER "stm32xx_hal.h"
    #endif
  #endif
#endif

#ifdef STLOG_HAL_HEADER
  #include STLOG_HAL_HEADER
#else
  /* Error: Please manually define STLOG_HAL_HEADER before including this file */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================
 *  User Configuration
 * ========================================================= */

/* 日志总开关：1=启用 0=禁用 */
#ifndef STLOG_ENABLE
#define STLOG_ENABLE 1
#endif

/* 模式选择：
 * 0 = FULL 模式（环形缓冲区 + DMA 自动发送）
 * 1 = MINI 模式（无缓冲区，直接阻塞发送，超低 Flash/RAM 占用）
 */
#ifndef STLOG_MINI
#define STLOG_MINI 0
#endif

/* 换行模式：1=输出 \r\n（Windows 风格） 0=仅 \n */
#ifndef STLOG_CRLF
#define STLOG_CRLF 1
#endif

/* 时间戳开关：1=在日志前添加 [tick_ms]（使用 HAL_GetTick） */
#ifndef STLOG_TIMESTAMP
#define STLOG_TIMESTAMP 0
#endif

/* 默认日志等级（0~4）：0=ERROR, 1=WARN, 2=INFO, 3=DEBUG, 4=TRACE */
#ifndef STLOG_DEFAULT_LEVEL
#define STLOG_DEFAULT_LEVEL 4 /* INFO */
#endif

/* 日志前缀自定义 */
#ifndef STLOG_PREFIX_E
#define STLOG_PREFIX_E "[E] "
#endif
#ifndef STLOG_PREFIX_W
#define STLOG_PREFIX_W "[W] "
#endif
#ifndef STLOG_PREFIX_I
#define STLOG_PREFIX_I "[I] "
#endif
#ifndef STLOG_PREFIX_D
#define STLOG_PREFIX_D "[D] "
#endif
#ifndef STLOG_PREFIX_T
#define STLOG_PREFIX_T "[T] "
#endif

/* FULL 模式环形缓冲区大小（字节数，越小越省 RAM） */
#ifndef STLOG_TX_BUF_SIZE
#define STLOG_TX_BUF_SIZE 512u
#endif

/* 缓冲区满时的处理策略：
 * 1 = 丢弃新数据（不阻塞，最稳定）
 * 0 = 覆盖旧数据（更实时）
 */
#ifndef STLOG_DROP_NEW_ON_FULL
#define STLOG_DROP_NEW_ON_FULL 1
#endif

/* ERROR/WARN 日志强制阻塞发送（推荐开启）：
 * 1 = ERROR/WARN 会阻塞等待发送完成，确保重要日志不丢失
 * 0 = 所有日志使用相同的发送策略
 * - FULL 模式：ERROR/WARN 会刷新缓冲区（必要时阻塞）
 * - MINI 模式：本身就是阻塞发送
 */
#ifndef STLOG_FORCE_BLOCKING_EW
#define STLOG_FORCE_BLOCKING_EW 1
#endif

/* 并发保护锁（防止中断/多线程输出混乱）
 * 自定义示例：
 *   #define STLOG_LOCK()   uint32_t pm=__get_PRIMASK(); __disable_irq();
 *   #define STLOG_UNLOCK() if(!pm) __enable_irq();
 */
#ifndef STLOG_LOCK
#define STLOG_LOCK()   do{}while(0)
#endif
#ifndef STLOG_UNLOCK
#define STLOG_UNLOCK() do{}while(0)
#endif

/* 时间戳来源（可自定义替换 HAL_GetTick） */
#ifndef STLOG_NOW_MS
#define STLOG_NOW_MS() ((unsigned)HAL_GetTick())
#endif

/* =========================================================
 *  Type Definitions
 * ========================================================= */
typedef enum {
    LOG_ERROR = 0,
    LOG_WARN  = 1,
    LOG_INFO  = 2,
    LOG_DEBUG = 3,
    LOG_TRACE = 4,
} log_level_t;

typedef struct {
    UART_HandleTypeDef *huart;
    volatile log_level_t level;

#if !STLOG_MINI
    volatile unsigned head;
    volatile unsigned tail;
    volatile unsigned tx_busy;
    unsigned char buf[STLOG_TX_BUF_SIZE];
#endif
} stlog_t;

/* Global singleton instance */
static stlog_t g_stlog;

/* =========================================================
 *  Internal Helper Functions (no stdlib dependency)
 * ========================================================= */

static inline void stlog_init(UART_HandleTypeDef *huart)
{
    g_stlog.huart = huart;
    g_stlog.level = (log_level_t)STLOG_DEFAULT_LEVEL;

#if !STLOG_MINI
    g_stlog.head = 0u;
    g_stlog.tail = 0u;
    g_stlog.tx_busy = 0u;
#endif
}

static inline void log_set_level(log_level_t lv)
{
    g_stlog.level = lv;
}

static inline unsigned stlog__lvl_enabled_(log_level_t lv)
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
    stlog_hex_u32_((unsigned)(unsigned long)p);
#else
    (void)p;
#endif
}

/* MINI mode doesn't need DMA callback */
static inline void stlog_on_tx_cplt(UART_HandleTypeDef *huart)
{
    (void)huart;
}

#else /* ---------- FULL Mode: Ring buffer + DMA auto-transmission ---------- */

/* Ring buffer helper functions */
static inline unsigned stlog__next_(unsigned x)
{
    x++;
    if (x >= (unsigned)STLOG_TX_BUF_SIZE) x = 0u;
    return x;
}

static inline unsigned stlog__seg_len_(const stlog_t *l)
{
    unsigned t = l->tail, h = l->head;
    return (h > t) ? (h - t) : ((unsigned)STLOG_TX_BUF_SIZE - t);
}

static inline void stlog__push_(stlog_t *l, unsigned char c)
{
    unsigned next = stlog__next_(l->head);

    if (next == l->tail) {
#if STLOG_DROP_NEW_ON_FULL
        return;
#else
        l->tail = stlog__next_(l->tail);
#endif
    }

    l->buf[l->head] = c;
    l->head = next;
}

static inline void stlog__flush_blocking_(stlog_t *l)
{
#if STLOG_ENABLE
    if (!l || !l->huart) return;
    while (l->head != l->tail) {
        unsigned t = l->tail;
        unsigned len = stlog__seg_len_(l);
        if (len == 0u) break;

        (void)HAL_UART_Transmit(l->huart, (uint8_t*)&l->buf[t], (uint16_t)len, 0xFFFFu);

        unsigned nt = t + len;
        if (nt >= (unsigned)STLOG_TX_BUF_SIZE) nt = 0u;
        l->tail = nt;
    }
#else
    (void)l;
#endif
}

/* Kick transmission: auto DMA if available, else blocking flush
 * Key: advance tail before DMA start to prevent duplicate transmission
 */
static inline void stlog__kick_(stlog_t *l)
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

        /* Advance tail first */
        unsigned nt = t + len;
        if (nt >= (unsigned)STLOG_TX_BUF_SIZE) nt = 0u;
        l->tail = nt;

        l->tx_busy = 1u;
        if (HAL_UART_Transmit_DMA(hu, (uint8_t*)&l->buf[t], (uint16_t)len) != HAL_OK) {
            /* DMA start failed: mark not busy (data already consumed -> dropped) */
            l->tx_busy = 0u;
        }
    } else {
        stlog__flush_blocking_(l);
    }
#else
    (void)l;
#endif
}

/* Must be called from HAL_UART_TxCpltCallback */
static inline void stlog_on_tx_cplt(UART_HandleTypeDef *huart)
{
#if STLOG_ENABLE
    if (g_stlog.huart == huart) {
        g_stlog.tx_busy = 0u;
        stlog__kick_(&g_stlog);
    }
#else
    (void)huart;
#endif
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
    stlog_hex_u32_((unsigned)(unsigned long)p);
#else
    (void)p;
#endif
}

#endif /* STLOG_MINI */

/* =========================================================
 *  Prefix and Timestamp Handling
 * ========================================================= */
static inline void stlog__emit_prefix_(log_level_t lv)
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
static inline void stlog__commit_level_(log_level_t lv)
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
        long:        stlog_i32_, \
        unsigned long: stlog_u32_, \
        long long:   stlog_i32_, \
        unsigned long long: stlog_u32_, \
        const void*: stlog_ptr_, \
        void*:       stlog_ptr_, \
        default:     stlog_ptr_ \
    )(x)

#if STLOG_ENABLE
  #define print(x)   do{ STLOG_LOCK(); stlog__print_any_(x); STLOG_UNLOCK(); }while(0)
  #define println(x) do{ STLOG_LOCK(); stlog__print_any_(x); stlog_nl_(); STLOG_UNLOCK(); }while(0)
#else
  #define print(x)   do{ (void)(x); }while(0)
  #define println(x) do{ (void)(x); }while(0)
#endif

/* =========================================================
 *  Level-based Logging Macros with Prefixes
 * ========================================================= */
#if STLOG_ENABLE
  #define loge(x) do{ if(stlog__lvl_enabled_(LOG_ERROR)){ STLOG_LOCK(); stlog__emit_prefix_(LOG_ERROR); stlog__print_any_(x); stlog_nl_(); stlog__commit_level_(LOG_ERROR); STLOG_UNLOCK(); } }while(0)
  #define logw(x) do{ if(stlog__lvl_enabled_(LOG_WARN )){ STLOG_LOCK(); stlog__emit_prefix_(LOG_WARN ); stlog__print_any_(x); stlog_nl_(); stlog__commit_level_(LOG_WARN ); STLOG_UNLOCK(); } }while(0)
  #define logi(x) do{ if(stlog__lvl_enabled_(LOG_INFO )){ STLOG_LOCK(); stlog__emit_prefix_(LOG_INFO ); stlog__print_any_(x); stlog_nl_(); stlog__commit_level_(LOG_INFO ); STLOG_UNLOCK(); } }while(0)
  #define logd(x) do{ if(stlog__lvl_enabled_(LOG_DEBUG)){ STLOG_LOCK(); stlog__emit_prefix_(LOG_DEBUG); stlog__print_any_(x); stlog_nl_(); stlog__commit_level_(LOG_DEBUG); STLOG_UNLOCK(); } }while(0)
  #define logt(x) do{ if(stlog__lvl_enabled_(LOG_TRACE)){ STLOG_LOCK(); stlog__emit_prefix_(LOG_TRACE); stlog__print_any_(x); stlog_nl_(); stlog__commit_level_(LOG_TRACE); STLOG_UNLOCK(); } }while(0)
#else
  #define loge(x) do{ (void)(x); }while(0)
  #define logw(x) do{ (void)(x); }while(0)
  #define logi(x) do{ (void)(x); }while(0)
  #define logd(x) do{ (void)(x); }while(0)
  #define logt(x) do{ (void)(x); }while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* STLOG_H */