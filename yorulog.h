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

/* Master logger switch: 1 = enabled, 0 = disabled */
#ifndef YORULOG_ENABLE
  #define YORULOG_ENABLE 1
#endif

/* Mode selection:
 * 0 = FULL mode (ring buffer + automatic DMA transmission)
 * 1 = MINI mode (no buffer, direct blocking transmit, ultra-low Flash/RAM usage)
 */
#ifndef YORULOG_MINI
  #define YORULOG_MINI 0
#endif

/* Newline mode: 1 = output \r\n (Windows style), 0 = output only \n */
#ifndef YORULOG_CRLF
  #define YORULOG_CRLF 1
#endif

/* Timestamp switch: 1 = prepend [tick_ms] using HAL_GetTick */
#ifndef YORULOG_TIMESTAMP
  #define YORULOG_TIMESTAMP 0
#endif

/* Optional Yoruassert integration:
 * 0 = no dependency
 * 1 = use Yoruassert for internal parameter/state assertions
 */
#ifndef YORULOG_USE_YORUASSERT
  #define YORULOG_USE_YORUASSERT 0
#endif

#if YORULOG_USE_YORUASSERT
  #ifndef YORULOG_YORUASSERT_HEADER
    #define YORULOG_YORUASSERT_HEADER "../Yoruassert/yoruassert.h"
  #endif

  /* Keep Yorulog -> Yoruassert integration one-way here to avoid recursive
   * header coupling with Yoruassert's optional Yorulog output path.
   */
  #ifndef YORUASSERT_USE_YORULOG
    #define YORUASSERT_USE_YORULOG 0
  #endif

  #include YORULOG_YORUASSERT_HEADER
#endif

/* Internal assert policy when YORULOG_USE_YORUASSERT = 1:
 * 1 = assert on invalid parameters / invalid runtime state
 * 0 = keep silent return behavior
 */
#ifndef YORULOG_ASSERT_ON_ERROR
  #define YORULOG_ASSERT_ON_ERROR 1
#endif

/* Callsite trace:
 * - PRINT: affects Println / PrintRawln
 * - LOG: affects LogTag / LogXxx / LogXxxTag
 */
#ifndef YORULOG_TRACE_PRINT_CALLSITE
  #define YORULOG_TRACE_PRINT_CALLSITE 0
#endif

#ifndef YORULOG_TRACE_LOG_CALLSITE
  #define YORULOG_TRACE_LOG_CALLSITE 0
#endif

/* Backward-compatible umbrella switch */
#ifndef YORULOG_TRACE_CALLSITE
  #define YORULOG_TRACE_CALLSITE 0
#endif

#if YORULOG_TRACE_CALLSITE
  #undef YORULOG_TRACE_PRINT_CALLSITE
  #define YORULOG_TRACE_PRINT_CALLSITE 1
  #undef YORULOG_TRACE_LOG_CALLSITE
  #define YORULOG_TRACE_LOG_CALLSITE 1
#endif

/* Default log level (0~4): 0=ERROR, 1=WARN, 2=INFO, 3=DEBUG, 4=TRACE */
#ifndef YORULOG_DEFAULT_LEVEL
  #define YORULOG_DEFAULT_LEVEL 4 /* TRACE */
#endif

/* Custom log prefixes */
#ifndef YORULOG_PREFIX_E
  #define YORULOG_PREFIX_E "[E] "
#endif
#ifndef YORULOG_PREFIX_W
  #define YORULOG_PREFIX_W "[W] "
#endif
#ifndef YORULOG_PREFIX_I
  #define YORULOG_PREFIX_I "[I] "
#endif
#ifndef YORULOG_PREFIX_D
  #define YORULOG_PREFIX_D "[D] "
#endif
#ifndef YORULOG_PREFIX_T
  #define YORULOG_PREFIX_T "[T] "
#endif

/* FULL mode ring buffer size in bytes; smaller values save RAM */
#ifndef YORULOG_TX_BUF_SIZE
  #define YORULOG_TX_BUF_SIZE 512u
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
 * - On H7, many projects place .data/.bss in DTCM by default, which DMA cannot access.
 *   For that reason, H7 enables DMA-accessible buffer support automatically.
 * - Other series keep this disabled by default to minimize integration surface;
 *   override it manually if needed.
 */
#ifndef YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER
  #if YORULOG_PLATFORM_STM32H7
    #define YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER 1
  #else
    #define YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER 0
  #endif
#endif

/* H7/F7-class platforms may have D-Cache; this tag can be used later to extend cache maintenance policy. */
#ifndef YORULOG_HAS_DCACHE_RISK
  #if YORULOG_PLATFORM_STM32H7 || YORULOG_PLATFORM_STM32F7
    #define YORULOG_HAS_DCACHE_RISK 1
  #else
    #define YORULOG_HAS_DCACHE_RISK 0
  #endif
#endif

/* Section used for the FULL mode DMA transmit buffer.
 * On STM32H7, .bss/.data are often placed in DTCM by default, which DMA cannot access.
 * Therefore the transmit buffer is placed into a DMA-accessible RAM_D2 section by default.
 */
#ifndef YORULOG_DMA_SECTION
  #define YORULOG_DMA_SECTION ".RAM_D2"
#endif

#if defined(__APPLE__)
#define YORULOG_SECTION_ATTR(name)
#elif YORULOG_NEEDS_DMA_ACCESSIBLE_BUFFER
#define YORULOG_SECTION_ATTR(name) __attribute__((section(name)))
#else
#define YORULOG_SECTION_ATTR(name)
#endif

/* D-Cache maintenance policy for DMA TX buffers.
 * On H7/F7-class Cortex-M7 parts, DMA may read stale data if the TX buffer lives in cacheable RAM.
 * Keep this enabled by default on those platforms unless the application guarantees a non-cacheable DMA region.
 */
#ifndef YORULOG_DMA_CACHE_CLEAN
  #define YORULOG_DMA_CACHE_CLEAN YORULOG_HAS_DCACHE_RISK
#endif

/* Cache line size used by SCB_CleanDCache_by_Addr alignment. */
#ifndef YORULOG_DMA_CACHE_LINE_SIZE
  #define YORULOG_DMA_CACHE_LINE_SIZE 32u
#endif

/* Buffer-full policy:
 * 1 = drop new data (non-blocking, most stable)
 * 0 = overwrite old data (more real-time)
 */
#ifndef YORULOG_DROP_NEW_ON_FULL
  #define YORULOG_DROP_NEW_ON_FULL 0
#endif

/* Block when the buffer is full until space becomes available:
 * 1 = prioritize log completeness without extra RAM; may block under peak load
 * 0 = keep legacy behavior (drop new or overwrite old depending on YORULOG_DROP_NEW_ON_FULL)
 *
 * Recommended for DMA logger setups such as STM32H7 when using a small buffer.
 */
#ifndef YORULOG_BLOCK_ON_FULL
  #define YORULOG_BLOCK_ON_FULL 1
#endif

/* Force blocking transmit for ERROR/WARN logs (recommended):
 * 1 = ERROR/WARN wait until transmission completes so critical logs are not lost
 * 0 = all levels use the same transmit strategy
 * - FULL mode: ERROR/WARN flush the buffer and may block if needed
 * - MINI mode: already blocking by design
 */
#ifndef YORULOG_FORCE_BLOCKING_EW
  #define YORULOG_FORCE_BLOCKING_EW 1
#endif

/* Concurrency protection lock to avoid mixed output from interrupts/threads.
 * Custom example:
 *   #define YORULOG_LOCK()   uint32_t pm=__get_PRIMASK(); __disable_irq();
 *   #define YORULOG_UNLOCK() if(!pm) __enable_irq();
 */
#ifndef YORULOG_LOCK
  #define YORULOG_LOCK()   do{}while(0)
#endif
#ifndef YORULOG_UNLOCK
  #define YORULOG_UNLOCK() do{}while(0)
#endif

/* Timestamp source; can be overridden instead of HAL_GetTick */
#ifndef YORULOG_NOW_MS
  #define YORULOG_NOW_MS() ((unsigned)HAL_GetTick())
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

#if !YORULOG_MINI
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
#if defined(YORULOG_DEFINE_GLOBALS)
YORULOG_HandleTypeDef hYorulog;
#if !YORULOG_MINI
YORULOG_SECTION_ATTR(YORULOG_DMA_SECTION) __attribute__((aligned(32)))
unsigned char yorulog_tx_buf[YORULOG_TX_BUF_SIZE];
#endif
#else
extern YORULOG_HandleTypeDef hYorulog;
#if !YORULOG_MINI
extern unsigned char yorulog_tx_buf[YORULOG_TX_BUF_SIZE];
#endif
#endif

#define g_stlog hYorulog
#if !YORULOG_MINI
#define g_stlog_tx_buf yorulog_tx_buf
#endif

#if YORULOG_USE_YORUASSERT && YORULOG_ASSERT_ON_ERROR
  #define stlog__assert_msg_(expr, msg) YORUASSERT_MSG((expr), (msg))
#else
  #define stlog__assert_msg_(expr, msg) do { (void)(expr); (void)(msg); } while (0)
#endif

/* =========================================================
 *  Internal Helper Functions (no stdlib dependency)
 * ========================================================= */

static inline void YORULOG_Init(UART_HandleTypeDef *huart)
{
    stlog__assert_msg_(huart != (UART_HandleTypeDef *)0, "yorulog init huart is null");
    g_stlog.huart = huart;
    g_stlog.level = (YORULOG_LevelTypeDef)YORULOG_DEFAULT_LEVEL;

#if !YORULOG_MINI
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
    stlog__assert_msg_((unsigned)lv <= (unsigned)YORULOG_LEVEL_TRACE, "yorulog level is out of range");
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
#if YORULOG_MINI

static inline void stlog__putc_(char c)
{
#if YORULOG_ENABLE
    stlog__assert_msg_(g_stlog.huart != (UART_HandleTypeDef *)0, "yorulog print before init");
    if (g_stlog.huart == (UART_HandleTypeDef *)0) return;
    (void)HAL_UART_Transmit(g_stlog.huart, (uint8_t*)&c, 1u, 0xFFFFu);
#else
    (void)c;
#endif
}

static inline void stlog_write_cstr_(const char *s)
{
#if YORULOG_ENABLE
    if (!s) return;
    while (*s) stlog__putc_(*s++);
#else
    (void)s;
#endif
}

static inline void stlog_nl_(void)
{
#if YORULOG_ENABLE
#if YORULOG_CRLF
    stlog__putc_('\r');
#endif
    stlog__putc_('\n');
#endif
}

static inline void stlog_write_cstr_long_(const char *s)
{
#if YORULOG_ENABLE
    stlog_write_cstr_(s);
#else
    (void)s;
#endif
}

static inline void stlog_nl_long_(void)
{
#if YORULOG_ENABLE
    stlog_nl_();
#endif
}

static inline void stlog_char_(char c)
{
#if YORULOG_ENABLE
    stlog__putc_(c);
#else
    (void)c;
#endif
}

static inline void stlog_u32_(unsigned v)
{
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
    if (x >= (unsigned)YORULOG_TX_BUF_SIZE) x = 0u;
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
    return (h > t) ? (h - t) : ((unsigned)YORULOG_TX_BUF_SIZE - t);
}

static inline void stlog__prepare_dma_tx_(const unsigned char *ptr, unsigned len)
{
#if YORULOG_ENABLE && YORULOG_DMA_CACHE_CLEAN
    if ((ptr == (const unsigned char *)0) || (len == 0u)) return;

    {
        const uintptr_t line_size = (uintptr_t)YORULOG_DMA_CACHE_LINE_SIZE;
        if (line_size == 0u) return;
        const uintptr_t start = (uintptr_t)ptr;
        const uintptr_t aligned_start = start & ~(line_size - 1u);
        const uintptr_t aligned_end = (start + (uintptr_t)len + line_size - 1u) & ~(line_size - 1u);

        if (aligned_end > aligned_start) {
            SCB_CleanDCache_by_Addr((uint32_t *)aligned_start, (int32_t)(aligned_end - aligned_start));
        }
    }
#else
    (void)ptr;
    (void)len;
#endif
}

static inline void stlog__tx_blocking_bytes_(const unsigned char *buf, unsigned len)
{
#if YORULOG_ENABLE
    stlog__assert_msg_(g_stlog.huart != (UART_HandleTypeDef *)0, "yorulog blocking tx before init");
    if (!buf || (len == 0u) || !g_stlog.huart) return;

    while (len != 0u) {
        uint16_t chunk = (uint16_t)((len > 0xFFFFu) ? 0xFFFFu : len);
        if (HAL_UART_Transmit(g_stlog.huart, (uint8_t *)buf, chunk, 0xFFFFu) != HAL_OK) {
            break;
        }
        buf += chunk;
        len -= (unsigned)chunk;
    }
#else
    (void)buf;
    (void)len;
#endif
}

static inline unsigned stlog__cstr_len_(const char *s)
{
    unsigned len = 0u;

    if (!s) return 0u;

    while (s[len] != '\0') {
        ++len;
    }

    return len;
}

static inline void stlog__flush_blocking_(YORULOG_HandleTypeDef *l);

static inline void stlog__push_(YORULOG_HandleTypeDef *l, unsigned char c)
{
    unsigned next = stlog__next_(l->head);

    if (next == l->tail) {
#if YORULOG_BLOCK_ON_FULL
        stlog__flush_blocking_(l);
        next = stlog__next_(l->head);
        if (next == l->tail) {
#if YORULOG_DROP_NEW_ON_FULL
            return;
#else
            l->tail = stlog__next_(l->tail);
#endif
        }
#elif YORULOG_DROP_NEW_ON_FULL
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
#if YORULOG_ENABLE
    stlog__assert_msg_(l != (YORULOG_HandleTypeDef *)0, "yorulog flush handle is null");
    if (l != (YORULOG_HandleTypeDef *)0) {
        stlog__assert_msg_(l->huart != (UART_HandleTypeDef *)0, "yorulog flush before init");
    }
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
#if YORULOG_ENABLE
    stlog__assert_msg_(l != (YORULOG_HandleTypeDef *)0, "yorulog kick handle is null");
    if (l != (YORULOG_HandleTypeDef *)0) {
        stlog__assert_msg_(l->huart != (UART_HandleTypeDef *)0, "yorulog kick before init");
    }
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
        stlog__prepare_dma_tx_(&l->tx_buf[t], len);
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
#if YORULOG_ENABLE
    stlog__assert_msg_(huart != (UART_HandleTypeDef *)0, "yorulog tx callback huart is null");
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
#if YORULOG_ENABLE
    if (!s) return;
    while (*s) stlog__push_(&g_stlog, (unsigned char)*s++);
    stlog__kick_(&g_stlog);
#else
    (void)s;
#endif
}

static inline void stlog_write_cstr_long_(const char *s)
{
#if YORULOG_ENABLE
    unsigned len;

    if (!s) return;

    len = stlog__cstr_len_(s);
#if !YORULOG_MINI
    stlog__flush_blocking_(&g_stlog);
#endif
    stlog__tx_blocking_bytes_((const unsigned char *)s, len);
#else
    (void)s;
#endif
}

static inline void stlog_nl_(void)
{
#if YORULOG_ENABLE
#if YORULOG_CRLF
    stlog__push_(&g_stlog, '\r');
#endif
    stlog__push_(&g_stlog, '\n');
    stlog__kick_(&g_stlog);
#endif
}

static inline void stlog_nl_long_(void)
{
#if YORULOG_ENABLE
#if YORULOG_CRLF
    static const unsigned char nl[] = "\r\n";
    stlog__tx_blocking_bytes_(nl, 2u);
#else
    static const unsigned char nl[] = "\n";
    stlog__tx_blocking_bytes_(nl, 1u);
#endif
#endif
}

static inline void stlog_char_(char c)
{
#if YORULOG_ENABLE
    stlog__push_(&g_stlog, (unsigned char)c);
    stlog__kick_(&g_stlog);
#else
    (void)c;
#endif
}

static inline void stlog_u32_(unsigned v)
{
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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
#if YORULOG_ENABLE
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

#endif /* YORULOG_MINI */

/* =========================================================
 *  Prefix and Timestamp Handling
 * ========================================================= */
static inline const char *stlog__file_name_(const char *path)
{
    const char *name = path;

    if (path == (const char *)0) return (const char *)0;

    while (*path != '\0') {
        if ((*path == '/') || (*path == '\\')) {
            name = path + 1;
        }
        ++path;
    }

    return name;
}

static inline void stlog__emit_callsite_(const char *file, unsigned line)
{
#if YORULOG_ENABLE
    const char *name = stlog__file_name_(file);

    stlog_write_cstr_(" (");
    stlog_write_cstr_(name ? name : "(null)");
    stlog_char_(':');
    stlog_u32_(line);
    stlog_char_(')');
#else
    (void)file;
    (void)line;
#endif
}

static inline void stlog__emit_prefix_(YORULOG_LevelTypeDef lv)
{
#if YORULOG_ENABLE
#if YORULOG_TIMESTAMP
    stlog_write_cstr_("[");
    stlog_u32_(YORULOG_NOW_MS());
    stlog_write_cstr_("] ");
#endif

    if (lv == LOG_ERROR) stlog_write_cstr_(YORULOG_PREFIX_E);
    else if (lv == LOG_WARN)  stlog_write_cstr_(YORULOG_PREFIX_W);
    else if (lv == LOG_INFO)  stlog_write_cstr_(YORULOG_PREFIX_I);
    else if (lv == LOG_DEBUG) stlog_write_cstr_(YORULOG_PREFIX_D);
    else stlog_write_cstr_(YORULOG_PREFIX_T);
#else
    (void)lv;
#endif
}

static inline void stlog__emit_tag_(const char *tag)
{
#if YORULOG_ENABLE
    if (!tag || (tag[0] == '\0')) return;
    stlog_write_cstr_("[");
    stlog_write_cstr_(tag);
    stlog_write_cstr_("] ");
#else
    (void)tag;
#endif
}

/* FULL mode: ERROR/WARN can force flush queue; MINI mode is always blocking */
static inline void stlog__commit_level_(YORULOG_LevelTypeDef lv)
{
#if YORULOG_ENABLE
#if !YORULOG_MINI
#if YORULOG_FORCE_BLOCKING_EW
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
#if YORULOG_ENABLE
#if YORULOG_MINI
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

static inline void YORULOG_PrintLong(const char *s)
{
#if YORULOG_ENABLE
    YORULOG_LOCK();
    stlog_write_cstr_long_(s ? s : "");
    YORULOG_UNLOCK();
#else
    (void)s;
#endif
}

static inline void YORULOG_PrintLongln(const char *s)
{
#if YORULOG_ENABLE
    YORULOG_LOCK();
    stlog_write_cstr_long_(s ? s : "");
    stlog_nl_long_();
    YORULOG_UNLOCK();
#else
    (void)s;
#endif
}

static inline void stlog_print_long(const char *s)
{
    YORULOG_PrintLong(s);
}

static inline void stlog_print_longln(const char *s)
{
    YORULOG_PrintLongln(s);
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

#define stlog__callsite_() stlog__emit_callsite_(__FILE__, (unsigned)__LINE__)

#if YORULOG_ENABLE
  #define YORULOG_Print(x)   do{ YORULOG_LOCK(); stlog__print_any_(x); YORULOG_UNLOCK(); }while(0)
  #define YORULOG_Println(x) do{ YORULOG_LOCK(); stlog__print_any_(x); if(YORULOG_TRACE_PRINT_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); }while(0)
  #define YORULOG_PrintRaw(x)   do{ YORULOG_LOCK(); stlog__print_any_(x); YORULOG_UNLOCK(); }while(0)
  #define YORULOG_PrintRawln(x) do{ YORULOG_LOCK(); stlog__print_any_(x); if(YORULOG_TRACE_PRINT_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); }while(0)
  #define print(x)   YORULOG_Print(x)
  #define println(x) YORULOG_Println(x)
#else
  #define YORULOG_Print(x)   do{ (void)(x); }while(0)
  #define YORULOG_Println(x) do{ (void)(x); }while(0)
  #define YORULOG_PrintRaw(x)   do{ (void)(x); }while(0)
  #define YORULOG_PrintRawln(x) do{ (void)(x); }while(0)
  #define print(x)   do{ (void)(x); }while(0)
  #define println(x) do{ (void)(x); }while(0)
#endif

/* =========================================================
 *  Level-based Logging Macros with Prefixes
 * ========================================================= */
#if YORULOG_ENABLE
  #define YORULOG_LogTag(tag, x) do{ YORULOG_LOCK(); stlog__emit_tag_(tag); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); }while(0)
  #define YORULOG_LogErrorTag(tag, x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_ERROR)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_ERROR); stlog__emit_tag_(tag); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_ERROR); } }while(0)
  #define YORULOG_LogWarnTag(tag, x)  do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_WARN )){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_WARN ); stlog__emit_tag_(tag); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_WARN ); } }while(0)
  #define YORULOG_LogInfoTag(tag, x)  do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_INFO )){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_INFO ); stlog__emit_tag_(tag); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_INFO ); } }while(0)
  #define YORULOG_LogDebugTag(tag, x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_DEBUG)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_DEBUG); stlog__emit_tag_(tag); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_DEBUG); } }while(0)
  #define YORULOG_LogTraceTag(tag, x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_TRACE)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_TRACE); stlog__emit_tag_(tag); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_TRACE); } }while(0)
  #define YORULOG_LogError(x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_ERROR)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_ERROR); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_ERROR); } }while(0)
  #define YORULOG_LogWarn(x)  do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_WARN )){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_WARN ); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_WARN ); } }while(0)
  #define YORULOG_LogInfo(x)  do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_INFO )){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_INFO ); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_INFO ); } }while(0)
  #define YORULOG_LogDebug(x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_DEBUG)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_DEBUG); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_DEBUG); } }while(0)
  #define YORULOG_LogTrace(x) do{ if(stlog__lvl_enabled_(YORULOG_LEVEL_TRACE)){ YORULOG_LOCK(); stlog__emit_prefix_(YORULOG_LEVEL_TRACE); stlog__print_any_(x); if(YORULOG_TRACE_LOG_CALLSITE) stlog__callsite_(); stlog_nl_(); YORULOG_UNLOCK(); stlog__commit_level_(YORULOG_LEVEL_TRACE); } }while(0)
  #define loge(x) YORULOG_LogError(x)
  #define logw(x) YORULOG_LogWarn(x)
  #define logi(x) YORULOG_LogInfo(x)
  #define logd(x) YORULOG_LogDebug(x)
  #define logt(x) YORULOG_LogTrace(x)
#else
  #define YORULOG_LogTag(tag, x) do{ (void)(tag); (void)(x); }while(0)
  #define YORULOG_LogErrorTag(tag, x) do{ (void)(tag); (void)(x); }while(0)
  #define YORULOG_LogWarnTag(tag, x)  do{ (void)(tag); (void)(x); }while(0)
  #define YORULOG_LogInfoTag(tag, x)  do{ (void)(tag); (void)(x); }while(0)
  #define YORULOG_LogDebugTag(tag, x) do{ (void)(tag); (void)(x); }while(0)
  #define YORULOG_LogTraceTag(tag, x) do{ (void)(tag); (void)(x); }while(0)
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
