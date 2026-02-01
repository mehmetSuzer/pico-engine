
# PICO-ENGINE

*A lightweight 3D graphics engine for embedded graphics.*

PICO-ENGINE is a project that I developed to push the limits of a microcontroller. It is a 3D graphics engine specifically designed for Raspberry Pi Pico.

The project has been built around the constraints of RP2040/RP2350 chips. The engine focuses on efficient use of limited CPU power and memory while still providing a functional 3D rendering pipeline. 
All core components including math routines, transformations, and rasterisation are implemented with performance and determinism in mind.

## ✨ Features

Designed for low-resource, deterministic, and real-time applications

Fixed-point arithmentic (Q_24_8, Q_16_16, Q8_24)

Adaptable to different screen resolutions

Support for different colour formats (RGB332, RGB565)

Support for different depth-bit lengths (DEPTH_8BIT, DEPTH_16BIT)

Adjustable swapchain (The default image count is 2)

Controllable camera

Typedefs for mesh, model, texture, and scene 

Parallel rendering (CORE0, CORE1, DMA)


## ⚙️ Configuration Macros

PICO-ENGINE allows several build-time options:

**CLOCK_FREQUENCY_KHZ**

- Enables overclocking/underclocking and adjusts the clock frequencies of the Pico.

**SCREEN_WIDTH** and **SCREEN_HEIGHT**

- Sets the screen resolution.

**Q8_24** or **Q16_16** or **Q24_8**

- Sets the fixed-point type.

**RGB332** or **RGB565**

- Sets the colour format of the fragments.

**DEPTH_8BIT** or **DEPTH_16BIT**

- Sets the depth-bit length of fragments.

## 🎥 Demo

A simple scene consisting of 7394 triangles:

https://github.com/user-attachments/assets/45e91a5e-157c-46c7-9ac0-30c936bbcc20

## ⚠️ Notes

The project is still under active development and testing, so some bugs may still be present.


## 📄 License
This project is licensed under the MIT License.  
See the [LICENSE](LICENSE) file for details.

