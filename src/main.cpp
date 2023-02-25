#include <loguru.hpp>

#include "psxemu.h"

constexpr auto DEFAULT_SCREEN_WIDTH = 1024;
constexpr auto DEFAULT_SCREEN_HEIGHT = 768;

int main(int argc, char* argv[])
{
    psxemu  emu;

    //Init Log Library
    loguru::init(argc, argv);
    loguru::g_stderr_verbosity = 1; 
    //loguru::add_file("debug.log", loguru::Append, 1);
    
    if (!emu.init(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT))
    {
        LOG_F(ERROR, "Failed to Initialize PSX Emulator!\n");
        return 0;
    }

    if (!emu.run())
    {
        LOG_F(ERROR, "Failed to Run PSP!\n");
        return 0;
    }
    
    return 0;
}