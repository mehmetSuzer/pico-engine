
#ifndef PICO_ENGINE_COLOUR_RGB332_H
#define PICO_ENGINE_COLOUR_RGB332_H

#include <stdint.h>

typedef uint8_t rgb332_t;

#define RGB332_RED_BITS    ((rgb332_t)0xE0u)
#define RGB332_GREEN_BITS  ((rgb332_t)0x1Cu)
#define RGB332_BLUE_BITS   ((rgb332_t)0x03u)

#define RGB332_BLACK       ((rgb332_t)0x00u)
#define RGB332_GREY        ((rgb332_t)0x92u)
#define RGB332_WHITE       ((rgb332_t)0xFFu)
#define RGB332_RED         ((rgb332_t)0xE0u)
#define RGB332_GREEN       ((rgb332_t)0x1Cu)
#define RGB332_BLUE        ((rgb332_t)0x03u)
#define RGB332_YELLOW      ((rgb332_t)0xFCu)
#define RGB332_MAGENTA     ((rgb332_t)0xE3u)
#define RGB332_CYAN        ((rgb332_t)0x1Fu)
#define RGB332_DARKGREY    ((rgb332_t)0x49u)
#define RGB332_DARKRED     ((rgb332_t)0x80u)
#define RGB332_DARKGREEN   ((rgb332_t)0x10u)
#define RGB332_DARKBLUE    ((rgb332_t)0x01u)
#define RGB332_LIGHTGREY   ((rgb332_t)0x86u)
#define RGB332_LIGHTRED    ((rgb332_t)0xD1u)
#define RGB332_LIGHTGREEN  ((rgb332_t)0x59u)
#define RGB332_LIGHTBLUE   ((rgb332_t)0x2Fu)
#define RGB332_ORANGE      ((rgb332_t)0xB6u) 
#define RGB332_PINK        ((rgb332_t)0xE6u) 
#define RGB332_PURPLE      ((rgb332_t)0x8Bu) 
#define RGB332_BROWN       ((rgb332_t)0xA8u) 
#define RGB332_TEAL        ((rgb332_t)0x17u) 
#define RGB332_LIME        ((rgb332_t)0xB4u)

#endif // PICO_ENGINE_COLOUR_RGB332_H

