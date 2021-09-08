#include "psxemu.h"

constexpr auto SCREEN_WIDTH = 1280;
constexpr auto SCREEN_HEIGHT = 1024;

int main(int argc, char* argv[])
{
    psxemu  emu;
    
    //Init PSX Emulator
    if (!emu.init(SCREEN_WIDTH, SCREEN_HEIGHT))
    {
        printf("Failed to Initialize SDL!\n");
        return 0;
    }

    if (!emu.run())
    {
        printf("Failed to Run PSP!\n");
        return 0;
    }
        
    return 0;
}