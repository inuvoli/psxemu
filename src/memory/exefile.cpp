#include <loguru.hpp>

#include "exefile.h"

exefile::exefile()
{
}

exefile::~exefile()
{
}

bool exefile::loadExe(const std::string& fileName, const uint8_t * memory)
{
    //Load EXE Header
	LOG_F(INFO, "EXE File (%s) Loading...", fileName.c_str());

	std::ifstream ifs;
	ifs.open(fileName, std::ifstream::binary);
	if (ifs.is_open())
	{
		ifs.seekg(0, ifs.beg);
		ifs.read((char*)header, EXE_HEADER_SIZE);

        //Check EXE Id
        std::string id = std::string(reinterpret_cast<char*>(header), 8);
        if (id.compare("PS-X EXE") != 0)
        {
            LOG_F(ERROR, "Invalid file identifier found!");
            return false;
        }

        //Load Header Parameters
        fileSize = loadFromHeader(0x1c);
        dstAddress = loadFromHeader(0x18);
        startProgramCounter = loadFromHeader(0x10);
        startGlobalPointer = loadFromHeader(0x14);
        startStackPointerBase = loadFromHeader(0x30);
        startStackPointerOffset = loadFromHeader(0x34);

        if (fileSize % 0x800 != 0)
        {
            LOG_F(ERROR, "Invalid file size!");
            return false;
        }

        // LOG_F(INFO, "EXE File Filesize 0x%08x", fileSize);
        // LOG_F(INFO, "EXE File Destination Address 0x%08x", dstAddress);
        // LOG_F(INFO, "EXE File Program Counter 0x%08x", startProgramCounter);
        // LOG_F(INFO, "EXE File Global Pointer 0x%08x", startGlobalPointer);
        // LOG_F(INFO, "EXE File Stack Pointer Base 0x%08x", startStackPointerBase);
        // LOG_F(INFO, "EXE File Stack Pointer Offset 0x%08x", startStackPointerOffset);

        //Load File to Memory    
        ifs.read((char*)(memory + (dstAddress & 0x1fffffff)), fileSize);
        ifs.close();

        LOG_F(INFO, "EXE File Loaded");
		return true;
	}
	LOG_F(ERROR, "PSP Bios Not Found");

    return false;
}

bool exefile::setRegisters(uint32_t * pc, uint32_t * gpr)
{
    *pc = startProgramCounter;
    gpr[28] = startGlobalPointer;

    if (startStackPointerBase != 0)
    {
        gpr[29] = startStackPointerBase + startStackPointerOffset;
        gpr[30] = startStackPointerBase + startStackPointerOffset;
    }
    
    return true;
}

uint32_t exefile::loadFromHeader(uint32_t offset)
{
    uint32_t result = 0;
    for (uint8_t i = 0; i < 4; i++) {
        result |= (((uint32_t)header[offset + i]) << (i * 8));
    }
    return result;
}