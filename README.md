# PSXemu - Another SDL3/OpenGL Playstation Emulator in C++ 20

----

PSXemu is a small Playstation emulator ment to be an exercise in SDL3 and OpenGL programming. It uses ImGui for debug widgets. It needs an original Playstation BIOS to run. Not running any game yet.

It can run CD Images and Executable files

```bash
psxemu.exe --bios <bios file path> --bin <cdrom image path>
psxemu.exe --bios <bios file path> --exe <executable path>
```

## What's working...
- CPU: full core and CP0 implementation, it pass all psxtest_cpu exe tests!
- GPU: full implementation, need to be tested
- RENDERER: Open GL Renderer full implementation, need to be tested
- SHADERS: Full PSX rendering pipeline emulation
- DMA: full implementation, need to be tested
- Timers: full implementation, need to be tested
- CONTROLLER: almost full implementation, need to be tested. Works with test exe, partially with Bios.
- CDROM: partial implementation

----
## TODOs...
- RENDERER: Support for Ring Buffer on Vertex Arrays
- CPU COP2/GTE implementation
- CONTROLLER: Some bug fixing needed for the Session FSM.
- SPU: To ddo
