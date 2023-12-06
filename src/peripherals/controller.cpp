#include <loguru.hpp>
#include "controller.h"
#include "psx.h"

//Cnstructor & Destructor
Controller::Controller()
{

}

Controller::~Controller()
{

}

//External Signals
bool Controller::execute()
{
    return true;
}

bool Controller::reset()
{
    return false;
}

//Internal Register Access
bool Controller::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{   
    switch (addr)
    {
    default:
        LOG_F(ERROR, "CONTROLLER - Unknown Parameter Set addr: 0x%08x (%d)", addr, bytes);
        break;
    }

    return true;
}

uint32_t Controller::readAddr(uint32_t addr, uint8_t bytes)
{
    uint32_t data = 0;

    switch (addr)
    {
    default:
        LOG_F(ERROR, "CONTROLLER - Unknown Parameter Get addr: 0x%08x (%d)", addr, bytes);
        break;
    }

    return data;
}

