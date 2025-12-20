#include <loguru.hpp>

#include "exefile.h"

exefile::exefile()
{
    //Reset Executable Informations
	exeInfo.isPresent = false;
    exeInfo.fileSize = 0x0;
    exeInfo.destinationAddress = 0x0;
    exeInfo.startProgramCounter = 0x0;
    exeInfo.startGlobalPointer = 0x0;
    exeInfo.startStackPointerBase = 0x0;
    exeInfo.startStackPointerOffset = 0x0; 
}

exefile::~exefile()
{
}

bool exefile::loadExe(const std::string& fileName, const uint8_t * memory)
{
    //Load EXE Header
	LOG_F(INFO, "EXE - File (%s) Loading...", fileName.c_str());

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
            LOG_F(ERROR, "EXE - Invalid file identifier found!");
            return false;
        }

        exeInfo.fileSize = loadFromHeader(0x1c);
        if (exeInfo.fileSize % 0x800 != 0)
        {
            LOG_F(ERROR, "EXE - Invalid file size!");
            return false;
        }

        //Load Exe Header Infos
        exeInfo.isPresent = true;
        exeInfo.destinationAddress = loadFromHeader(0x18);
        exeInfo.startProgramCounter = loadFromHeader(0x10);
        exeInfo.startGlobalPointer = loadFromHeader(0x14);
        exeInfo.startStackPointerBase = loadFromHeader(0x30);
        exeInfo.startStackPointerOffset = loadFromHeader(0x34);     
           
        LOG_F(INFO, "EXE - File Filesize 0x%08x", exeInfo.fileSize);
        LOG_F(INFO, "EXE - File Destination Address 0x%08x", exeInfo.destinationAddress);
        LOG_F(INFO, "EXE - File Program Counter 0x%08x", exeInfo.startProgramCounter);
        LOG_F(INFO, "EXE - File Global Pointer 0x%08x", exeInfo.startGlobalPointer);
        LOG_F(INFO, "EXE - File Stack Pointer Base 0x%08x", exeInfo.startStackPointerBase);
        LOG_F(INFO, "EXE - File Stack Pointer Offset 0x%08x", exeInfo.startStackPointerOffset);

        //Load File to Memory    
        ifs.read((char*)(memory + (exeInfo.destinationAddress & 0x1fffffff)), exeInfo.fileSize);
        ifs.close();

        LOG_F(INFO, "EXE - File Loaded");
		return true;
	}
	LOG_F(ERROR, "EXE - File Not Found");

    return false;
}

bool exefile::setRegisters(uint32_t * pc, uint32_t * gpr)
{
    if (!exeInfo.isPresent)
    {
        LOG_F(ERROR, "EXE - No EXE Loaded, cannot set registers!");
        return false;
    }

    *pc = exeInfo.startProgramCounter;
    gpr[28] = exeInfo.startGlobalPointer;

    if (exeInfo.startStackPointerBase != 0)
    {
        gpr[29] = exeInfo.startStackPointerBase + exeInfo.startStackPointerOffset;
        gpr[30] = exeInfo.startStackPointerBase + exeInfo.startStackPointerOffset;
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