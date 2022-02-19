#include "debugger.h"

Debugger::Debugger(std::shared_ptr<Psx> instance)
{
    //TODO
    psx = instance;
    breakPoint = 0xffffffff;
    //breakPoint = 0x800585c4;
    //breakPoint = 0xa000b9b0;
    //breakPoint = 0xbfc06f18;
    //breakPoint = 0xa0010000;
    //breakPoint = 0x1f000084;
    //breakPoint = 0x1f802021;
    //breakPoint = 0x80000080;
    //breakPoint = 0x1f801801;
    //breakPoint = 0x00000001;
    //breakPoint = 0xbfc80000;
    //breakPoint = 0x80030828;
    
    stepMode = StepMode::Halt;
};

Debugger::~Debugger()
{
    //TODO
};

void Debugger::toggleDebugModuleStatus(DebugModule module)
{
    int index = static_cast<int>(module);

    //Toggle Module Debug Status
    debugModuleStatus[index] = (debugModuleStatus[index]) ? false : true;
};

bool Debugger::getDebugModuleStatus(DebugModule module)
{
    int index = static_cast<int>(module);

    return debugModuleStatus[index];
};

bool Debugger::update()
{
    return true;
};