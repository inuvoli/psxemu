#include <loguru.hpp>

#include "controller.h"
#include "psx.h"

//Cnstructor & Destructor
Controller::Controller()
{
    //Reset Registers
    statRegister.word = 0;
    modeRegister.word = 0;
    ctrlRegister.word = 0;
    baudRegister = 0;
    txData = 0;
    rxFifo.flush();
}

Controller::~Controller()
{
}

//External Signals
bool Controller::execute()
{
    //Update Baudrate Timer
    statRegister.baudratetimer = statRegister.baudratetimer - 1;
    if (statRegister.baudratetimer == 0)
        statRegister.baudratetimer = baudCounter;

    return true;
}

bool Controller::reset()
{
    //Reset Registers
    statRegister.word = 0;
    modeRegister.word = 0;
    ctrlRegister.word = 0;
    baudRegister = 0;
    txData = 0;
    rxFifo.flush();

    return true;
}

//Internal Register Access
bool Controller::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{   
    controller::StatusRegister statusReg;

    switch (addr)
    {
        case 0x1f801040:    //Write JOY_TX_DATA Register
            txData = data & 0x000000ff;
            LOG_F(3, "CTR - Write JOY_TX_DATA Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f801048:
            modeRegister.word = static_cast<uint16_t>(data) & 0x013f;
            LOG_F(INFO, "CTR - Write JOY_MODE Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f80104a:    //Write JOY_CTRL Register
            ctrlRegister.word = static_cast<uint16_t>(data) & 0x3f7f;
            LOG_F(INFO, "CTR - Write JOY_CTRL Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f80104e:    //Write JOY_BAUD Register
            baudRegister = static_cast<uint16_t>(data);
            switch(modeRegister.baudmult)
            {
                case 0:
                case 1:
                    baudCounter = (baudRegister * 1) / 2;
                    break;
                case 2:
                    baudCounter = (baudRegister * 16) / 2;
                    break;
                case 3:
                    baudCounter = (baudRegister * 64) / 2;
                    break;
            }
            if (baudCounter == 0)
                baudCounter = 1; //avoid division by zero in further calculations
            statRegister.baudratetimer = baudCounter;
            LOG_F(INFO, "CTR - Write JOY_BAUD Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        default:
            LOG_F(ERROR, "CTR - Write Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            return false;
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
            if (rxFifo.pop(tmp))
                data = tmp;
            LOG_F(INFO, "CTR - Read JOY_RX_DATA Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f801044:
            data = statRegister.word;
            LOG_F(3, "CTR - Read JOY_STAT Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f801048:
            data = modeRegister.word;
            LOG_F(INFO, "CTR - Read JOY_MODE Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;
        case 0x1f80104a:
            data = ctrlRegister.word;
            LOG_F(INFO, "CTR - Read JOY_CTRL Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;
        case 0x1f80104e:    //Read JOY_BAUD Register
            data = baudRegister;
            LOG_F(INFO, "CTR - Read JOY_BAUD Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        default:
            LOG_F(ERROR, "CTR - Read Unknown Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            return false;
            break;
    }

    return data;
}

