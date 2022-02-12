#include "debugger.h"

Debugger::Debugger(std::shared_ptr<Psx> instance)
{
    //TODO
    psx = instance;
    breakPoint = 0xffffffff;
    //breakPoint = 0x800585c4;
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