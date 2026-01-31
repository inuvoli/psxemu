#include <loguru.hpp>

#include "controller.h"
#include "psx.h"

//Cnstructor & Destructor
Controller::Controller()
{
    //Create State Machine
    stateMachine = std::make_unique<ControllerStateMachine>(this);

    //Reset Initial State
    reset();
}

Controller::~Controller()
{
}

bool Controller::reset()
{
    //Reset Registers
    statRegister.word = 0;
    statRegister.txready2 = 1; //TX Ready
    statRegister.ackinputlevel = 1; //ACK Active Low
    modeRegister.word = 0;
    ctrlRegister.word = 0;
    baudRegister = 0;
    rxFifo.flush();

    //Reset Controller States
    for (int i = 0; i < 2; i++)
    {
        controllerState[i].buttons = 0xffff;     //All buttons released
        controllerState[i].analogL = 0x80;       //Analog Stick Centered
        controllerState[i].analogR = 0x80;       //Analog Stick Centered
        controllerState[i].analogT = 0x00;       //Analog Triggers Released
        controllerState[i].enabled = false;      //Controller Disabled
    }

    //Reset Helper Variables
    baudCounter = 0;

    return true;
}

//External Signals
bool Controller::execute()
{
    //Update Baudrate Timer & Internal Clock
    statRegister.baudratetimer = statRegister.baudratetimer - 1;
    if (statRegister.baudratetimer == 0)
    {
        statRegister.baudratetimer = baudCounter;
        stateMachine->pushEvent(ControllerEvent::CONTROLLER_CLOCK_TICK_EVENT);
    }

    //Only Used for ACK Signal Pulsing
    stateMachine->pushEvent(ControllerEvent::CPU_CLOCK_TICK_EVENT);
        
    return true;
}

//-------------------------------------------------------------------------------------------------------------
//
// Internal Registers Read and Write Functions
//
//-------------------------------------------------------------------------------------------------------------
bool Controller::writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes)
{   
    controller::StatusRegister statusReg;

    switch (addr)
    {
        case 0x1f801040:    //Write JOY_TX_DATA Register
            txLatch = data & 0x000000ff;
            stateMachine->pushEvent(ControllerEvent::TX_COMMAND_RECEIVED_EVENT, txLatch);
            LOG_F(INFO, "CTR - Write JOY_TX_DATA Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f801048:    //Write JOY_MODE Register
            modeRegister.word = static_cast<uint16_t>(data) & 0x013f;
            LOG_F(INFO, "CTR - Write JOY_MODE Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f80104a:    //Write JOY_CTRL Register
            ctrlRegister.word = static_cast<uint16_t>(data) & 0x3f7f;
            //Enable or Disable Controllers based on JOY_CTRL Bits
            controllerState[0].enabled = (ctrlRegister.slot == 0 && ctrlRegister.joyoutput == 1);
            controllerState[1].enabled = (ctrlRegister.slot == 1 && ctrlRegister.joyoutput == 1);
            //Enable JOY_STAT.bit0 based on TX Enable, RX Enable, Joy SEL and JOY_STAT.bit2
            if (ctrlRegister.txen == 1 && ctrlRegister.joyoutput == 1 && ctrlRegister.rxen == 0 && statRegister.txready2 == 1)
                stateMachine->pushEvent(ControllerEvent::START_TX_SESSION_EVENT);
            //Check for SEL Disable to end any TX Session
            if (ctrlRegister.joyoutput == 0)
                stateMachine->pushEvent(ControllerEvent::END_TX_SESSION_EVENT);
            //Acnowkedge JOY_STAT.bit3 and bit9 if requested
            if (ctrlRegister.ack == 1)
            {
                statRegister.rxparityerror = 0;
                statRegister.interruptreq = 0;
            }
            LOG_F(INFO, "CTR - Write JOY_CTRL Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f80104e:    //Write JOY_BAUD Register
            baudRegister = static_cast<uint16_t>(data);
            switch(modeRegister.baudmult)
            {
                case 0:
                case 1:
                    baudCounter = baudRegister;
                    break;
                case 2:
                    baudCounter = baudRegister * 16;
                    break;
                case 3:
                    baudCounter = baudRegister * 64;
                    break;
            }
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
        case 0x1f801040:    //Read JOY_RX_DATA Register
            uint8_t tmp;
            if (!rxFifo.isempty())
            {
                rxFifo.pop(tmp);
                data = tmp;
            }
            else
                data = 0xff; //No Data Available
			stateMachine->pushEvent(ControllerEvent::RX_RESPONSE_SENT_EVENT);
            LOG_F(INFO, "CTR - Read JOY_RX_DATA Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f801044:    //Read JOY_STAT Register
            data = statRegister.word;
            LOG_F(INFO, "CTR - Read JOY_STAT Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;

        case 0x1f801048:    //Read JOY_MODE Register
            data = modeRegister.word;
            LOG_F(INFO, "CTR - Read JOY_MODE Register:\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
            break;
        case 0x1f80104a:    //Read JOY_CTRL Register
            data = ctrlRegister.word;
            LOG_F(INFO, "CTR - Read JOY_CTRL Register:\t\t0x%08x (%d), data: 0x%08x", addr, bytes, data);
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
//-------------------------------------------------------------------------------------------------------------
//
// Controller Status Update Functions
//
//-------------------------------------------------------------------------------------------------------------
void Controller::setButtonState(uint8_t controllerID, ControllerButton buttonMask, bool pressed)
{
    
    if (pressed)
        controllerState[controllerID].buttons &= ~(1U << static_cast<uint16_t>(buttonMask));
    else
        controllerState[controllerID].buttons |= (1U << static_cast<uint16_t>(buttonMask));

	LOG_F(3, "CTR - Set Controller %d State to: 0x%04x", controllerID + 1, controllerState[controllerID].buttons);    
}

void Controller::setLeftAnalogStickX(uint8_t controllerID, int x)
{
    uint8_t clampedX = static_cast<uint8_t>((x + 32768) >> 8); //Convert from -32768..32767 to 0..255
    if (clampedX > 0x80 - CONTROLLER_DEADZONE && 
        clampedX < 0x80 + CONTROLLER_DEADZONE)
		clampedX = 0x80; //Apply Deadzone

    controllerState[controllerID].analogL &= 0xff00; 
    controllerState[controllerID].analogL |= clampedX;

	LOG_F(3, "CTR - Set Controller %d Left Analog Stick X to: 0x%04x", controllerID + 1, controllerState[controllerID].analogL);
}

void Controller::setLeftAnalogStickY(uint8_t controllerID, int y)
{
    uint8_t clampedY = static_cast<uint8_t>((y + 32768) >> 8); //Convert from -32768..32767 to 0..255
    if (clampedY > 0x80 - CONTROLLER_DEADZONE &&
        clampedY < 0x80 + CONTROLLER_DEADZONE)
        clampedY = 0x80; //Apply Deadzone

    controllerState[controllerID].analogL &= 0x00ff;
    controllerState[controllerID].analogL |= (clampedY << 8);

    LOG_F(3, "CTR - Set Controller %d Left Analog Stick Y to: 0x%04x", controllerID + 1, controllerState[controllerID].analogL);
}

void Controller::setRightAnalogStickX(uint8_t controllerID, int x)
{
    uint8_t clampedX = static_cast<uint8_t>((x + 32768) >> 8); //Convert from -32768..32767 to 0..255
    if (clampedX > 0x80 - CONTROLLER_DEADZONE &&
        clampedX < 0x80 + CONTROLLER_DEADZONE)
        clampedX = 0x80; //Apply Deadzone

    controllerState[controllerID].analogR &= 0xff00;
    controllerState[controllerID].analogR |= clampedX;

	LOG_F(3, "CTR - Set Controller %d Right Analog Stick X to: 0x%04x", controllerID + 1, controllerState[controllerID].analogR);
}

void Controller::setRightAnalogStickY(uint8_t controllerID, int y)
{
    uint8_t clampedY = static_cast<uint8_t>((y + 32768) >> 8); //Convert from -32768..32767 to 0..255
    if (clampedY > 0x80 - CONTROLLER_DEADZONE &&
        clampedY < 0x80 + CONTROLLER_DEADZONE)
        clampedY = 0x80; //Apply Deadzone

    controllerState[controllerID].analogR &= 0x00ff;
    controllerState[controllerID].analogR |= (clampedY << 8);

	LOG_F(3, "CTR - Set Controller %d Right Analog Stick Y to: 0x%04x", controllerID + 1, controllerState[controllerID].analogR);
}

void Controller::setLeftTrigger(uint8_t controllerID, int value)
{
    uint8_t clampedL = static_cast<uint8_t>(value >> 7); //Convert from 0..32767 to 0..255
    
    controllerState[controllerID].analogT &= 0xff00;
    controllerState[controllerID].analogT |= clampedL;

    LOG_F(3, "CTR - Set Controller %d Left Trigger to: 0x%04x", controllerID + 1, controllerState[controllerID].analogT);
}

void Controller::setRightTrigger(uint8_t controllerID, int value)
{
    uint8_t clampedR = static_cast<uint8_t>(value >> 7); //Convert from 0..32767 to 0..255

    controllerState[controllerID].analogT &= 0x00ff;
    controllerState[controllerID].analogT |= (clampedR << 8);

    LOG_F(3, "CTR - Set Controller %d Right Trigger to: 0x%04x", controllerID + 1, controllerState[controllerID].analogT);
}

//-------------------------------------------------------------------------------------------------------------
//
// Controller State Machine Implementation
//
//-------------------------------------------------------------------------------------------------------------
ControllerStateMachine::ControllerStateMachine(Controller *ptr)
{
    controller = std::shared_ptr<Controller>(ptr);
    currentState = ControllerState::IDLE;
}

ControllerStateMachine::~ControllerStateMachine()
{
    //Nothing to do
}

void ControllerStateMachine::pushEvent(ControllerEvent event, uint8_t eventData)
{
    static int ackPulseCounter = 0;
    static int baudTickCounter = 0;
    static int currentCommand = 0;

    //------------------------------------------------------------------------------------------------------- HANDLE ACK PULSE
    if (event == ControllerEvent::CPU_CLOCK_TICK_EVENT)
    {
        if (ackPulseCounter > 0)
        {
            ackPulseCounter--;
            if (ackPulseCounter == 0)
            {
                LOG_F(INFO, "CTR - ACK Pulse Ended");
                controller->statRegister.ackinputlevel = 1; //ACK Inactive High
                controller->statRegister.rxready = 1;
            }
        }
    }
    //------------------------------------------------------------------------------------------------------- HANDLE ACK PULSE

    switch(currentState)
    {
        //--------------------------------------------------------------------------------------------------- IDLE STATE
        case ControllerState::IDLE:
        switch (event)
        {
            case ControllerEvent::START_TX_SESSION_EVENT:
                controller->statRegister.txready1 = 1; //Enable TX Ready
                LOG_F(INFO, "CTR - START_TX_SESSION_EVENT received, moving to SESSION_STARTED state");
                currentState = ControllerState::SESSION_STARTED;
                break;

            default:
                break;
        }   
        break;
        //-------------------------------------------------------------------------------------------------- IDLE STATE

        //-------------------------------------------------------------------------------------------------- SESSION_STARTED STATE
        case ControllerState::SESSION_STARTED:
        switch(event)
        {
            case ControllerEvent::TX_COMMAND_RECEIVED_EVENT:
                baudTickCounter = CMD_TRANSMISSION_DURATION;    
                controller->statRegister.txready1 = 0;
                //Set Interrupt Request if TX Interrupt is Enabled
                if (controller->ctrlRegister.txintenable == 1)
                {
                    controller->statRegister.interruptreq = 1;
                    controller->psx->interrupt->request(static_cast<uint32_t>(interrupt::Cause::sio));
                    LOG_F(INFO, "CTR - IRQ7 Requested due to TX Interrupt Enable");
                }
                currentCommand = eventData;
                if (currentCommand == 0x01) //Digital Controller Command
                    loadCommandResponse();
                LOG_F(INFO, "CTR - TX_COMMAND_RECEIVED_EVENT received [0x%02x], moving to START_SEND_CMD_TO_PAD state", currentCommand);
                currentState = ControllerState::START_SEND_CMD_TO_PAD;
                break;

            case ControllerEvent::END_TX_SESSION_EVENT:
                controller->statRegister.txready1 = 0;
                currentState = ControllerState::IDLE;
                LOG_F(INFO, "CTR - END_TX_SESSION_EVENT received, moving to IDLE state");
                break;

            default:
                break;
        }
        break;
        //-------------------------------------------------------------------------------------------------- SESSION_STARTED STATE

        //-------------------------------------------------------------------------------------------------- START_SEND_CMD_TO_PAD STATE
        case ControllerState::START_SEND_CMD_TO_PAD:
        switch(event)
        {
            case ControllerEvent::CONTROLLER_CLOCK_TICK_EVENT:
                baudTickCounter--;
                controller->statRegister.txready2 = 0;
                LOG_F(INFO, "CTR - Transmitting Command to Pad [0x%02x], moving to SENDING_CMD_TO_PAD state", currentCommand);
                currentState = ControllerState::SENDING_CMD_TO_PAD;
                break;

            case ControllerEvent::END_TX_SESSION_EVENT:
                baudTickCounter = 0;
                currentState = ControllerState::IDLE;
                LOG_F(INFO, "CTR - END_TX_SESSION_EVENT received, moving to IDLE state");
                break;

            default:
                break;
        }
        break;
        //-------------------------------------------------------------------------------------------------- START_SEND_CMD_TO_PAD STATE
        
        //-------------------------------------------------------------------------------------------------- SENDING_CMD_TO_PAD STATE
        case ControllerState::SENDING_CMD_TO_PAD:
        switch(event)
        {
            case ControllerEvent::CONTROLLER_CLOCK_TICK_EVENT:
                baudTickCounter--;
                if (baudTickCounter == 0)
                {
                    baudTickCounter = CMD_TRANSMISSION_DURATION;
                    controller->statRegister.txready2 = 1; 
                    LOG_F(INFO, "CTR - Command Sent to Pad: 0x%02x, moving to START_RECV_RX_FROM_PAD state", currentCommand);
                    currentState = ControllerState::START_RECV_RX_FROM_PAD;
                }
                break;

            case ControllerEvent::END_TX_SESSION_EVENT:
                baudTickCounter = 0;
                controller->statRegister.txready2 = 1; 
                currentState = ControllerState::IDLE;
                LOG_F(INFO, "CTR - END_TX_SESSION_EVENT received, moving to IDLE state");
                break;

            default:
                break;
        }
        break;
        //-------------------------------------------------------------------------------------------------- SENDING_CMD_TO_PAD STATE

        //-------------------------------------------------------------------------------------------------- START_RECV_RX_FROM_PAD STATE
        case ControllerState::START_RECV_RX_FROM_PAD:
        switch(event)
        {
            case ControllerEvent::CONTROLLER_CLOCK_TICK_EVENT:
                baudTickCounter--;
                if (baudTickCounter == 0)
                {                     
                    //Trigger Ack Pulse for all but last byte of 0x00 command
                    if (controller->rxFifo.length() > 1)
                    {
                        controller->statRegister.ackinputlevel = 0; //ACK Active Low
                        ackPulseCounter = ACK_SIGNAL_DURATION;
                        LOG_F(INFO, "CTR - ACK Pulse Started");
                    }
                    else
                    {
                        //No Ack for last byte of 0x00 command, set RX Ready immediately
                        controller->statRegister.rxready = 1;
                    }
                    
                    //Set Interrupt Request if ACK or RX Interrupts are Enabled
                    if (controller->ctrlRegister.ackintenable == 1 || controller->ctrlRegister.rxintenable == 1)
                    {
                        controller->statRegister.interruptreq = 1;
                        controller->psx->interrupt->request(static_cast<uint32_t>(interrupt::Cause::sio));
                        LOG_F(INFO, "CTR - IRQ7 Requested due to RX or ACK Interrupt Enable");
                    }
                    LOG_F(INFO, "CTR - Response Received from Pad for Command: 0x%02x, moving to END_RECV_RX_FROM_PAD state", currentCommand);
                    currentState = ControllerState::END_RECV_RX_FROM_PAD;
                }
                break;

            case ControllerEvent::END_TX_SESSION_EVENT:
                baudTickCounter = 0;
                currentState = ControllerState::IDLE;
                LOG_F(INFO, "CTR - END_TX_SESSION_EVENT received, moving to IDLE state");
                break;

            default:
                break;
        }
        break;
        //-------------------------------------------------------------------------------------------------- START_RECV_RX_FROM_PAD STATE

        //-------------------------------------------------------------------------------------------------- END_RECV_RX_FROM_PAD STATE
        case ControllerState::END_RECV_RX_FROM_PAD:
        switch(event)
        {
            case ControllerEvent::RX_RESPONSE_SENT_EVENT:
                controller->statRegister.txready1 = 1;
                controller->statRegister.rxready = 0;
                currentState = ControllerState::SESSION_STARTED;
                LOG_F(INFO, "CTR - RX_RESPONSE_SENT_EVENT received, moving to SESSION_STARTED state");
                break;

                case ControllerEvent::END_TX_SESSION_EVENT:
                controller->statRegister.rxready = 0;
                controller->rxFifo.flush(); 
                currentState = ControllerState::IDLE;
                LOG_F(INFO, "CTR - END_TX_SESSION_EVENT received, moving to IDLE state");
                break;

            default:
                break;
        }
        //-------------------------------------------------------------------------------------------------- END_RECV_RX_FROM_PAD STATE
        break;
    }
}

void ControllerStateMachine::loadCommandResponse()
{
    //Load Response bytes for Digital Controller (only this one supported)
    controller->rxFifo.push(0xFF); //Start Byte
    controller->rxFifo.push(0x41); //Controller ID Byte (Digital Controller)
    controller->rxFifo.push(0x5A); //Constant Byte
    for(int i = 0; i < 2; i++)
    {
        if (controller->controllerState[i].enabled)
        {
            controller->rxFifo.push(static_cast<uint8_t>(controller->controllerState[i].buttons & 0x00FF));       //Button Low Byte
            controller->rxFifo.push(static_cast<uint8_t>((controller->controllerState[i].buttons >> 8) & 0x00FF)); //Button High Byte
        }
    }
}
