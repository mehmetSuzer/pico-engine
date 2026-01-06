
#ifndef PICO_ENGINE_COMMON_DEPTH_H
#define PICO_ENGINE_COMMON_DEPTH_H

#include <stdint.h>

#if !defined(DEPTH_8BIT) && !defined(DEPTH_16BIT)
    #define DEPTH_8BIT
    #warning "No depth format is specified, defaulting to DEPTH_8BIT."
#endif

#if defined(DEPTH_8BIT)
    #define depth_t uint8_t
    #define DEPTH_NEAREST  ((uint8_t)0x00u)
    #define DEPTH_FURTHEST ((uint8_t)0xFFu)
#elif defined(DEPTH_16BIT)
    #define depth_t uint16_t
    #define DEPTH_NEAREST  ((uint8_t)0x0000u)
    #define DEPTH_FURTHEST ((uint8_t)0xFFFFu)
#endif

#endif // PICO_ENGINE_COMMON_DEPTH_H

