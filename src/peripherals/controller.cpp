#include <loguru.hpp>

#include "controller.h"
#include "psx.h"

//Cnstructor & Destructor
Controller::Controller()
{
    joyStat = 0x00000005;
    joyMode = 0x0000000d;
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
    joyStat = 0x00000005;
    joyMode = 0x0000000d;

    return true;
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
        case 0x1f801040:
            uint8_t tmp;
            if (rxfifo.pop(tmp))
                data = tmp;
            LOG_F(INFO, "CONTROLLER - Read Joy RX Data:   0x%08x", data);
            break;

        case 0x1f801044:
            data = joyStat;
            LOG_F(INFO, "CONTROLLER - Read Joy Stat:   0x%08x, [0x%08x]", data, psx->cpu->pc);
            break;

        case 0x1f801048:
            data = joyMode;
            LOG_F(INFO, "CONTROLLER - Read Joy Mode:   0x%08x", data);
            break;

        case 0x1f80104c:
            LOG_F(INFO, "CONTROLLER - Read Unknoen register: 0x%08x", data);
            break;

    default:
        LOG_F(ERROR, "CONTROLLER - Unknown Parameter Get addr: 0x%08x (%d)", addr, bytes);
        break;
    }

    return data;
}

