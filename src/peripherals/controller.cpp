#include "controller.h"

//Cnstructor & Destructor
Controller::Controller()
{

}

Controller::~Controller()
{

}

//External Signals
bool Controller::clock()
{
    return false;
}

bool Controller::reset()
{
    return false;
}

//Internal Register Access
bool Controller::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{
    return false;
}

uint32_t Controller::readAddr(uint32_t addr, uint8_t bytes)
{
    return 0x0;
}

