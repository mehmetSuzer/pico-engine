
#ifndef PICO_ENGINE_COMMON_QTYPE_H
#define PICO_ENGINE_COMMON_QTYPE_H

#if !defined(Q8_24) && !defined(Q24_8) && !defined(Q16_16)
    #define Q16_16
    #warning "No fixed-point data type has been specified, defaulting to q16_16_t."
#endif

#if defined(Q16_16)
    #include <qglm/q16_16_glm.h>
#elif defined(Q8_24)
    #include <qglm/q8_24_glm.h>
#elif defined(Q24_8)
    #include <qglm/q24_8_glm.h>
#endif

#endif // PICO_ENGINE_COMMON_QTYPE_H

