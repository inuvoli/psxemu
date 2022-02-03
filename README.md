# PSXemu - Another SDL2/OpenGL Playstation Emulator in C++ 20

----

PSXemu is a small Playstation emulator ment to be an exercise in SDL2 and OpenGL programming. It uses ImGui for debug widgets. It needs an original Playstation BIOS to run, tested with scph1001.bin version.

## What's working...
- CPU: full core and CP0 implementation
- GPU: partial implementation, no Line and Rectangle rendering
- DMA: fully implemented
- Timers: fully implemented

----
## TODOs...
- GPU Texture management and missing GP0 commands (Line and Rectangle rendering)
- CPU COP2/GTE implementation
- CDROM Support
- Controllers Implementation
- SPU Implementation
- Emulator is really slow (15fps) improve clock timing routine
