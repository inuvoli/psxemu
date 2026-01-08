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

// Sector information structure
struct SectorInfo {
    uint8_t mm;
    uint8_t ss;
    uint8_t ff;
    uint8_t mode;
    uint8_t form;
    uint32_t lba;
};

class CdImage
{
public:
    CdImage();
    ~CdImage();

    //Interface Methods
    bool openImage(const std::string& fileName);
    bool closeImage();

    void setLocation(uint8_t min, uint8_t sec, uint8_t frac);
    bool seekSector();
    int readSector(char* buf);

    // Getter methods
    uint32_t getSectorOffset() const { return sectorOffset; }
    uint32_t getSectorTarget() const { return sectorTarget; }
    uint8_t getImageMode() const { return sectorInfo.mode; }
    uint8_t getImageForm() const { return sectorInfo.form; }
    const SectorInfo& getSectorInfo() const { return sectorInfo; }

private:
    // Helper methods
    uint8_t bcdToDecimal(uint8_t bcd);
    uint32_t calculateLBA(uint8_t mm, uint8_t ss, uint8_t ff);
    bool readSectorHeader();

    std::string fileName;
    std::ifstream image;

    SectorInfo sectorInfo;      // First sector information
    uint32_t sectorOffset;      // LBA offset from first sector
    uint32_t sectorTarget;      // Target sector LBA, updated by setLocation
};