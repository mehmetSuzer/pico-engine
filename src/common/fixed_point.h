
#ifndef PICO_ENGINE_COMMON_FIXED_POINT_H
#define PICO_ENGINE_COMMON_FIXED_POINT_H

#define FIXED_POINT_TYPE_COUNT \
    (defined(Q8_24) + defined(Q16_16) + defined(Q24_8))

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

#if defined(Q8_24)
    #include <qglm/q8_24_glm.h>
#elif defined(Q16_16)
    #include <qglm/q16_16_glm.h>
#elif defined(Q24_8)
    #include <qglm/q24_8_glm.h>
#endif

#endif // PICO_ENGINE_COMMON_QTYPE_H

