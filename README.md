# PSXemu - SDL2/OpenGL Playstation Emulator in C++ 20

PSXemu is a smal experimental emulator project for the Playstation 1. It aims to be an easy to use and well-documented emulator. Currently it is able to complete the BIOS startup with proper rendering.

#Features
A full implementation of the MIPS R3000A CPU.
Partial GPU implementation with texturing.
Working DMA routines.
Software rasterizer for accurate rendering.
Goals
Create a JIT recompiler in the future.
Complete the GPU (support for line primitives).
Boot the first game.
Create an easy to use GUI.

'psxemu' needs the original Playstation Bios to run. Copy a dump of the original ROM (scph-1001.bin) in /bin
