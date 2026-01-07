
#ifndef PICO_ENGINE_PGL_DEPTH_H
#define PICO_ENGINE_PGL_DEPTH_H

#include "common/fixed_point.h"

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

static inline depth_t depth_map(Q_TYPE depth, Q_TYPE near, Q_TYPE far)
{
    const int32_t bit_range = (int32_t)(DEPTH_FURTHEST - DEPTH_NEAREST);
    const Q_TYPE ratio = q_div(q_sub(depth, near), q_sub(far, near));
    const depth_t depth_bit = Q_TO_INT(q_add(q_mul_int(ratio, bit_range), Q_FROM_INT(DEPTH_NEAREST)));
    return depth_bit;
}

#endif // PICO_ENGINE_PGL_DEPTH_H

