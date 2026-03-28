#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <fstream>
#include <memory>

#include "litelib.h"

constexpr auto EXE_HEADER_SIZE = 0x800;

struct ExeInfo
{
	bool 		isPresent;
	uint32_t	fileSize;
	uint32_t	destinationAddress;
	uint32_t	startProgramCounter;
	uint32_t	startGlobalPointer;
	uint32_t	startStackPointerBase;
    uint32_t    startStackPointerOffset;
};

class exefile
{
public:
    exefile();
    ~exefile();

    bool loadExe(const std::string& fileName, const uint8_t * memory);
    bool setRegisters(uint32_t * pc, uint32_t * gpr);

private:
    uint8_t     header[EXE_HEADER_SIZE];
    ExeInfo     exeInfo;
    
    uint32_t    loadFromHeader(uint32_t offset);

};