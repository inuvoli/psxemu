#include "libcdimage.h"

CdImage::CdImage()
{
    imageMode = 0;
    imageForm = 0;
    mmOffset = 0;
    ssOffset = 0;
    fracOffset = 0;
    sectorOffset = 0;
    sectorTarget  = 0;
}

bool CdImage::openImage(const std::string& fileName)
{
    //Check file extension
    std::string _fileExtension;
    std::string _fileName;
    
    //Separate Filename and Extension
    _fileName = fileName.substr(0, fileName.find("."));
    _fileExtension = fileName.substr(fileName.find(".") + 1, 3);

    //Check for supported file extensions
    if (_fileExtension == "cue")
    {
        image.open(_fileName + ".bin", std::ifstream::binary);
    }
    else if (_fileExtension == "bin")
    {
        image.open(_fileName + "." + _fileExtension, std::ifstream::binary);
    }
    else if (_fileExtension == "img")
    {
        image.open(_fileName + "." + _fileExtension, std::ifstream::binary);
    }
    else
    {
        //File Extension not Supported
        return false;
    }

    //If CDROM Image File was open succesfully, Check for the First Sector Header
    if (!image.fail())
    {
        //Check Header and SubHeader for Mode2 Sector
        char sectorHeader[sector_header_size];
        image.seekg(0, image.beg);
        image.read(sectorHeader, sector_header_size);
        mmOffset = sectorHeader[12];
        ssOffset = sectorHeader[13];
        fracOffset = sectorHeader[14];
        imageMode = sectorHeader[15];
        sectorOffset = (mmOffset * 60 + ssOffset) * 75 + fracOffset;

        char sectorSubHeader[sector_subheader_size];
        uint8_t subMode;
        image.read(sectorSubHeader, sector_subheader_size);
        subMode = (uint8_t)sectorSubHeader[18];
        imageForm = (subMode & 0x20) ? 2 : 1;

        image.seekg(0, image.beg);      
    }

    return !image.fail();
}

bool CdImage::closeImage()
{
    image.close();
    return !image.fail();
}

bool CdImage::setLocation(int min, int sec, int frac)
{
    sectorTarget = (min * 60 + sec) * 75 + frac;
    return true;
}

bool CdImage::seekSector()
{
    //Seek Sector in Cd Image
    image.seekg(sector_size*(sectorTarget - sectorOffset), image.beg);
    return !image.fail();
}

bool CdImage::readSector(char* buf)
{
    char sectorHeader[sector_header_size];
    char sectorSubHeader[sector_subheader_size];
    char sectorTail[sector_tail_size];

    //Read Sector starting from current position
     image.read(sectorHeader, sector_header_size);
     image.read(sectorSubHeader, sector_subheader_size);
     image.read(buf, sector_payload_size);
     image.read(sectorTail, sector_tail_size);
    return !image.fail();
}
