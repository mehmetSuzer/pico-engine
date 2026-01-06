
#ifndef PICO_ENGINE_COMMON_MACROS_H
#define PICO_ENGINE_COMMON_MACROS_H

#define STRINGIFY(var)      #var
#define UNUSED(x)           (void)(x)
#define COUNT_OF(arr)       (sizeof(arr) / sizeof((arr)[0]))
#define FOR_EACH(i, arr)    for (size_t i = 0; i < COUNT_OF(arr); ++i)

#define CONTAINER_OF(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))

#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

#define IS_POWER_OF_2(x)   (((x) != 0) && (((x) & ((x) - 1)) == 0))

#define ALIGN_UP(x, align)      (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(x, align)    ((x) & ~((align) -1))

#define SIGN(x)             (((x) > 0) - ((x) < 0))
#define SMALLER(x, y)       (((x) < (y)) ? (x) :  (y))
#define GREATER(x, y)       (((x) > (y)) ? (x) :  (y))
#define ABS(x)              (((x) >= 0)  ? (x) : -(x))
#define CLAMP(x, min, max)  (((x) < (min)) ? (min) : ((x) > (max)) ? (max) : (x))
#define INTERP(x, y, a)     ((y) + (a) * ((x) - (y)))

#define SWAP(ptr1, ptr2)                        \
    {                                           \
        const typeof(*(ptr1)) temp = *(ptr1);   \
        *(ptr1) = *(ptr2);                      \
        *(ptr2) = temp;                         \
    }

#define BIT(n)              (1u << (n))
#define SET_BIT(x, n)       ((x) |= BIT(n))
#define CLEAR_BIT(x, n)     ((x) &= ~BIT(n))
#define TOGGLE_BIT(x, n)    ((x) ^= BIT(n))
#define CHECK_BIT(x, n)     (!!((x) & BIT(n)))

#endif // PICO_ENGINE_COMMON_MACROS_H

