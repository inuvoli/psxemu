#include "psxemu.h"

constexpr auto DEFAULT_SCREEN_WIDTH = 1024;
constexpr auto DEFAULT_SCREEN_HEIGHT = 768;

int main(int argc, char* argv[])
{
    psxemu  emu;
    
    //Init PSX Emulator
    if (!emu.init(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT))
    {
        printf("Failed to Initialize PSX Emulator!\n");
        return 0;
    }

    if (!emu.run())
    {
        printf("Failed to Run PSP!\n");
        return 0;
    }
    
    return 0;
}