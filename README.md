# PSXemu - Another SDL2/OpenGL Playstation Emulator in C++ 20

----

PSXemu is a small Playstation emulator ment to be an exercise in SDL3 and OpenGL programming. It uses ImGui for debug widgets. It needs an original Playstation BIOS to run, tested with scph1001.bin version.

## What's working...
- CPU: full core and CP0 implementation, it pass all psxtest_cpu exe tests!
- GPU: partial implementation, no Line and Rectangle rendering
- DMA: fully implemented, need to be tested
- Timers: fully implemented, need to be tested
- CDROM: Partial implementation

----
## TODOs...
- GPU Texture management and missing GP0 commands (Line and Rectangle rendering)
- CPU COP2/GTE implementation
- Controllers Implementation
- SPU Implementation
- Emulator is really slow (30fps) improve clock timing routine
