#pragma once

#include <cstdint>
#include <cstdio>
#include <string> 

namespace interrupt
{
    enum class Cause : uint8_t
    {
        vblank = 0x00,
        gpu = 0x01,
        cdrom = 0x02,
        dma = 0x03,
        timer0 = 0x04,
        timer1 = 0x05,
        timer2 = 0x06,
        sio = 0x07,
        spu = 0x09,
        pio = 0x0a
    };
}


class Psx;

class Interrupt
{
public:
    Interrupt();
    ~Interrupt();

    bool reset();
    bool execute();

    bool request(uint32_t cause);

    bool writeAddr(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t readAddr(uint32_t addr, uint8_t bytes);
   
    //Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

    //Getter & Setters
    uint32_t getStatusRegister() const { return i_stat; }
    void     setStatusRegister(uint32_t value) { i_stat = value; }
    uint32_t getMaskRegister() const { return i_mask; }
    void     setMaskRegister(uint32_t value) { i_mask = value; }   

public:
    uint32_t    i_stat;     //Interrupt Status Register
    uint32_t    i_mask;     //Interrupt Mask Register

private:
    //Link to PSP Object
	Psx* psx;

    

    std::string interruptDescription[11] = { "vBlank", "GPU", "CDROM", "DMA", "Timer0", "Timer1", "Timer2", "ByteRecv", "SIO", "SPU", "Controller" };
};
