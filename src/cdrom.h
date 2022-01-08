#pragma once

#include <cstdint>
#include <cstdio>

class Psx;

class Cdrom
{
public:
	Cdrom();
	~Cdrom();

	bool reset();
	bool clock();

	bool setParameter(uint32_t addr, uint32_t& data, uint8_t bytes);
	uint32_t getParameter(uint32_t addr, uint8_t bytes);

public:
	//Connect to PSX Instance
	void link(Psx* instance) { psx = instance; }

private:
	//Link to Bus Object
	Psx* psx = nullptr;	
};

