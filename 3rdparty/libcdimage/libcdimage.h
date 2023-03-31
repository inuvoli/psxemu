#pragma once

#include <cstdint>
#include <string>
#include <fstream>

constexpr auto sector_size = 2352;	//2352 bytes

constexpr auto sector_header_size = 16;
constexpr auto sector_subheader_size = 8;
constexpr auto sector_payload_size = 2048;
constexpr auto sector_tail_size = 280;

constexpr auto sector_payload_mode1_size = 2048;
constexpr auto sector_tail_mode1_size = 288;
constexpr auto sector_payload_mode2_size = 2336;


class CdImage
{
public:
    CdImage();
    ~CdImage() {};

    //Interface Methods
    bool openImage(const std::string& fileName);
    bool closeImage();

    bool setLocation(int min, int sec, int frac);
    bool readSector(char* buf);
    bool seekSector();

private:
    uint32_t seekTarget;    //Expressed in number of sectors
    std::string fileName;
    std::ifstream image;

    uint8_t     imageMode;
    uint8_t     imageForm;
    uint8_t     mmOffset;
    uint8_t     ssOffset;
    uint8_t     fracOffset;
    uint32_t    sectorOffset;       //Contains the Offset in byte of the first sector
    uint32_t    sectorTarget;       //Contains sector specified by setLocation
};