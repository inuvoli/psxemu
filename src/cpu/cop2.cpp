#include "cop2.h"
#include "cpu_short_pipe.h"

Cop2::Cop2(CPU* instance)
{
	//Link cop2 to cpu
	cpu = instance;
	
    //Reset Cop2 Registers
    std::memset(reg, 0x00, sizeof(uint32_t) * 64);
};

Cop2::~Cop2()
{

};

bool Cop2::reset()
{
    //Reset Cop0 Registers
    std::memset(reg, 0x00, sizeof(uint32_t) * 64);

	return true;
}

bool Cop2::execute(uint32_t cofun)
{
	printf("COP2 - Error! GTE is unsupported\n");
	return false;
}