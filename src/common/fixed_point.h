
#ifndef PICO_ENGINE_COMMON_FIXED_POINT_H
#define PICO_ENGINE_COMMON_FIXED_POINT_H

#ifdef Q8_24
    #define HAS_Q8_24 1
#else
    #define HAS_Q8_24 0
#endif

#ifdef Q16_16
    #define HAS_Q16_16 1
#else
    #define HAS_Q16_16 0
#endif

#ifdef Q24_8
    #define HAS_Q24_8 1
#else
    #define HAS_Q24_8 0
#endif

#define FIXED_POINT_TYPE_COUNT (HAS_Q8_24 + HAS_Q16_16 + HAS_Q24_8)

#if FIXED_POINT_TYPE_COUNT == 0
    #define Q16_16
    #warning "No fixed-point data type specified, defaulting to Q16_16."
#elif FIXED_POINT_TYPE_COUNT > 1
    #undef Q8_24
    #undef Q16_16
    #undef Q24_8
    #define Q16_16
    #warning "Multiple fixed-point data types specified, defaulting to Q16_16."
#endif

#undef FIXED_POINT_TYPE_COUNT
#undef HAS_Q8_24
#undef HAS_Q16_16
#undef HAS_Q24_8

// ----------------------------------------------------------------------------------- //

#if defined(Q8_24)
    #include <qglm/q8_24_glm.h>
#elif defined(Q16_16)
    #include <qglm/q16_16_glm.h>
#elif defined(Q24_8)
    #include <qglm/q24_8_glm.h>
#endif

#endif // PICO_ENGINE_COMMON_QTYPE_H

