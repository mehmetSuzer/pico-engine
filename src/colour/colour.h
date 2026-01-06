
#ifndef PICO_ENGINE_COLOUR_COLOUR_H
#define PICO_ENGINE_COLOUR_COLOUR_H

#include "rgb332.h"
#include "rgb565.h"

#define COLOUR_FORMAT_COUNT \
    (defined(RGB332) + defined(RGB565))

#if COLOUR_FORMAT_COUNT == 0
    #define RGB332
    #warning "No colour format specified, defaulting to RGB332."
#elif COLOUR_FORMAT_COUNT > 1
    #undef RGB332
    #undef RGB565
    #define RGB332
    #warning "Multiple colour formats specified, defaulting to RGB332."
#endif

#undef COLOUR_FORMAT_COUNT

#if defined(RGB332)

    typedef rgb332_t colour_t;

    #define COLOUR_RED_BITS     RGB332_RED_BITS
    #define COLOUR_GREEN_BITS   RGB332_GREEN_BITS
    #define COLOUR_BLUE_BITS    RGB332_BLUE_BITS

    #define COLOUR_BLACK        RGB332_BLACK       
    #define COLOUR_GREY         RGB332_GREY        
    #define COLOUR_WHITE        RGB332_WHITE       
    #define COLOUR_RED          RGB332_RED         
    #define COLOUR_GREEN        RGB332_GREEN       
    #define COLOUR_BLUE         RGB332_BLUE        
    #define COLOUR_YELLOW       RGB332_YELLOW      
    #define COLOUR_MAGENTA      RGB332_MAGENTA     
    #define COLOUR_CYAN         RGB332_CYAN        
    #define COLOUR_DARKGREY     RGB332_DARKGREY    
    #define COLOUR_DARKRED      RGB332_DARKRED     
    #define COLOUR_DARKGREEN    RGB332_DARKGREEN   
    #define COLOUR_DARKBLUE     RGB332_DARKBLUE    
    #define COLOUR_LIGHTGREY    RGB332_LIGHTGREY   
    #define COLOUR_LIGHTRED     RGB332_LIGHTRED    
    #define COLOUR_LIGHTGREEN   RGB332_LIGHTGREEN  
    #define COLOUR_LIGHTBLUE    RGB332_LIGHTBLUE   
    #define COLOUR_ORANGE       RGB332_ORANGE      
    #define COLOUR_PINK         RGB332_PINK        
    #define COLOUR_PURPLE       RGB332_PURPLE      
    #define COLOUR_BROWN        RGB332_BROWN       
    #define COLOUR_TEAL         RGB332_TEAL        
    #define COLOUR_LIME         RGB332_LIME        

    #define COLOUR_HALF         rgb332_half
    #define COLOUR_AVERAGE      rgb332_average

#elif defined(RGB565)

    typedef rgb565_t colour_t;
    
    #define COLOUR_RED_BITS     RGB565_RED_BITS
    #define COLOUR_GREEN_BITS   RGB565_GREEN_BITS
    #define COLOUR_BLUE_BITS    RGB565_BLUE_BITS

    #define COLOUR_BLACK        RGB565_BLACK       
    #define COLOUR_GREY         RGB565_GREY        
    #define COLOUR_WHITE        RGB565_WHITE       
    #define COLOUR_RED          RGB565_RED         
    #define COLOUR_GREEN        RGB565_GREEN       
    #define COLOUR_BLUE         RGB565_BLUE        
    #define COLOUR_YELLOW       RGB565_YELLOW      
    #define COLOUR_MAGENTA      RGB565_MAGENTA     
    #define COLOUR_CYAN         RGB565_CYAN        
    #define COLOUR_DARKGREY     RGB565_DARKGREY    
    #define COLOUR_DARKRED      RGB565_DARKRED     
    #define COLOUR_DARKGREEN    RGB565_DARKGREEN   
    #define COLOUR_DARKBLUE     RGB565_DARKBLUE    
    #define COLOUR_LIGHTGREY    RGB565_LIGHTGREY   
    #define COLOUR_LIGHTRED     RGB565_LIGHTRED    
    #define COLOUR_LIGHTGREEN   RGB565_LIGHTGREEN  
    #define COLOUR_LIGHTBLUE    RGB565_LIGHTBLUE   
    #define COLOUR_ORANGE       RGB565_ORANGE      
    #define COLOUR_PINK         RGB565_PINK        
    #define COLOUR_PURPLE       RGB565_PURPLE      
    #define COLOUR_BROWN        RGB565_BROWN       
    #define COLOUR_TEAL         RGB565_TEAL        
    #define COLOUR_LIME         RGB565_LIME        

    #define COLOUR_HALF         rgb565_half
    #define COLOUR_AVERAGE      rgb565_average

#endif

#endif // PICO_ENGINE_COLOUR_COLOUR_H

