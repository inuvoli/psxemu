#include "libcdimage.h"

CdImage::CdImage()
{
    sectorOffset = 0;
    sectorTarget = 0;
    sectorInfo.mm = 0;
    sectorInfo.ss = 0;
    sectorInfo.ff = 0;
    sectorInfo.mode = 0;
    sectorInfo.form = 0;
    sectorInfo.lba = 0;
}

CdImage::~CdImage()
{
    closeImage();
}

uint8_t CdImage::bcdToDecimal(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint32_t CdImage::calculateLBA(uint8_t mm, uint8_t ss, uint8_t ff)
{
    return (mm * 60 + ss) * 75 + ff;
}

bool CdImage::readSectorHeader()
{
    char header[sector_header_size];
    char subheader[sector_subheader_size];

    image.seekg(0, std::ios::beg);
    image.read(header, sector_header_size);
    
    if (image.fail() || image.gcount() != sector_header_size)
        return false;

    // Extract header information
    sectorInfo.mm = header[12];
    sectorInfo.ss = header[13];
    sectorInfo.ff = header[14];
    sectorInfo.mode = header[15];
    sectorInfo.lba = calculateLBA(bcdToDecimal(sectorInfo.mm), 
                                   bcdToDecimal(sectorInfo.ss), 
                                   bcdToDecimal(sectorInfo.ff));
    sectorOffset = sectorInfo.lba;

    // Read subheader for Mode 2 detection
    image.read(subheader, sector_subheader_size);
    
    if (image.fail() || image.gcount() != sector_subheader_size)
        return false;

    uint8_t subMode = static_cast<uint8_t>(subheader[2]);
    sectorInfo.form = (subMode & 0x20) ? 2 : 1;

    return true;
}

bool CdImage::openImage(const std::string& fileName)
{
    // Close any previously open image
    closeImage();

    // Extract extension
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos == std::string::npos)
        return false;

    std::string extension = fileName.substr(dotPos + 1);
    std::string baseName = fileName.substr(0, dotPos);

    // Determine actual image file to open
    std::string imageFile;
    
    if (extension == "cue")
        imageFile = baseName + ".bin";
    else if (extension == "bin" || extension == "img")
        imageFile = fileName;
    else
        return false; // Unsupported format

    // Open the image file
    image.open(imageFile, std::ios::binary | std::ios::in);
    
    if (!image.is_open() || image.fail())
        return false;

    // Read and parse first sector header
    if (!readSectorHeader())
    {
        closeImage();
        return false;
    }

    this->fileName = imageFile;
    return true;
}

bool CdImage::closeImage()
{
    if (image.is_open())
    {
        image.close();
    }
    return !image.fail();
}

void CdImage::setLocation(uint8_t min, uint8_t sec, uint8_t frac)
{
    // Convert BCD to decimal
    uint8_t minDec = bcdToDecimal(min);
    uint8_t secDec = bcdToDecimal(sec);
    uint8_t fracDec = bcdToDecimal(frac);

    // Calculate target LBA
    uint32_t targetLBA = calculateLBA(minDec, secDec, fracDec);
    
    // Calculate actual sector position
    sectorTarget = targetLBA;
}

bool CdImage::seekSector()
{
    if (!image.is_open())
        return false;

    // Calculate file position based on target sector and offset
    uint32_t sectorIndex = sectorTarget - sectorOffset;
    std::streampos position = static_cast<std::streampos>(sectorIndex) * sector_size;

    image.seekg(position, std::ios::beg);
    
    return !image.fail();
}

int CdImage::readSector(char* buf)
{
    if (!image.is_open() || buf == nullptr)
        return 0;

    // Read full sector into temporary buffer
    char sectorData[sector_size];
    
    image.read(sectorData, sector_size);
    
    if (image.fail())
        return 0;

    int bytesRead = static_cast<int>(image.gcount());
    
    //if read less than full sector, return 0
    if (bytesRead < sector_size)
        return 0;

    // Determine payload offset based on mode
    int payloadOffset;
    if (sectorInfo.mode == 1)
    {
        // Mode 1: payload starts after header only
        payloadOffset = sector_header_size;
    }
    else
    {
        // Mode 2: payload starts after header and subheader
        payloadOffset = sector_header_size + sector_subheader_size;
    }

    // Copy exactly sector_payload_size bytes
    std::copy(sectorData + payloadOffset, 
              sectorData + payloadOffset + sector_payload_size, 
              buf);

    return sector_payload_size;
}
