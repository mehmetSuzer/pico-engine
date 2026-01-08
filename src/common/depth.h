
#ifndef PICO_ENGINE_COMMON_DEPTH_H
#define PICO_ENGINE_COMMON_DEPTH_H

#include <stdint.h>

#ifdef DEPTH_8BIT
    #define HAS_DEPTH_8BIT 1
#else
    #define HAS_DEPTH_8BIT 0
#endif

#ifdef DEPTH_16BIT
    #define HAS_DEPTH_16BIT 1
#else
    #define HAS_DEPTH_16BIT 0
#endif

#define DEPTH_FORMAT_COUNT (HAS_DEPTH_8BIT + HAS_DEPTH_16BIT)

#if DEPTH_FORMAT_COUNT == 0
    #define DEPTH_8BIT
    #warning "No depth format specified, defaulting to DEPTH_8BIT."
#elif DEPTH_FORMAT_COUNT > 1
    #undef DEPTH_8BIT
    #undef DEPTH_16BIT
    #define DEPTH_8BIT
    #warning "Multiple depth formats specified, defaulting to DEPTH_8BIT."
#endif

#undef DEPTH_FORMAT_COUNT
#undef HAS_DEPTH_8BIT
#undef HAS_DEPTH_16BIT

// ----------------------------------------------------------------------------------- //

#if defined(DEPTH_8BIT)
    typedef uint8_t depth_t;
    #define DEPTH_NEAREST  ((depth_t)0x00u)
    #define DEPTH_FURTHEST ((depth_t)0xFFu)
#elif defined(DEPTH_16BIT)
    typedef uint16_t depth_t;
    #define DEPTH_NEAREST  ((depth_t)0x0000u)
    #define DEPTH_FURTHEST ((depth_t)0xFFFFu)
#endif

#define DEPTH_RANGE (DEPTH_FURTHEST - DEPTH_NEAREST)

#endif // PICO_ENGINE_COMMON_DEPTH_H

