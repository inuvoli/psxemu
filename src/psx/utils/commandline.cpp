#include <loguru.hpp>
#include "commandline.h"

bool commandline::checkCommand(char** begin, char** end, const std::string &cmd)
{
    return std::find(begin, end, cmd) != end;
}

char* commandline::getStringValue(char** begin, char** end, const std::string &cmd)
{
    char** p = std::find(begin, end, cmd);
    
    if (p != end && p++ != end)
    {
        return *p;
    }

    return nullptr;
}

int commandline::getIntValue(char** begin, char** end, const std::string &cmd)
{
    return 0;
}

bool commandline::parse(int argc, char* argv[])
{
    if (argc <= 1)
        return false;

    //Parse all command line parameters
    if (checkCommand(argv, argv + argc, "--help"))
    {
        LOG_F(INFO, "usage: psxemu [--version] [--help]");
        LOG_F(INFO, "              [--bios <bios filename]");
        LOG_F(INFO, "              [--exe <exe filename]");
        LOG_F(INFO, "              [--bin <bin filename]");
        return false;
    }

    if (checkCommand(argv, argv + argc, "--version"))
    {
        LOG_F(INFO, "psxemu version 0.1 (alpha)");
        return false;
    }

    if (checkCommand(argv, argv + argc, "--bios"))
    {
        char *filename = getStringValue(argv, argv + argc, "--bios");
        if (filename != nullptr)
        {
            biosFilename = std::string(filename);
        }
        else
        {
            LOG_F(ERROR, "Incorrect Bios filename parameter!");
            return false;
        }
    }

    if (checkCommand(argv, argv + argc, "--exe"))
    {
        char *filename = getStringValue(argv, argv + argc, "--exe");
        if (filename != nullptr)
        {
            exeFilename = std::string(filename);
        }
        else
        {
            LOG_F(ERROR, "Incorrect Exe filename parameter!");
            return false;
        }
        
    }

    if (checkCommand(argv, argv + argc, "--bin"))
    {
        char *filename = getStringValue(argv, argv + argc, "--bin");
        if (filename != nullptr)
        {
            binFilename = std::string(filename);
        }
        else
        {
            LOG_F(ERROR, "Incorrect Bin filename parameter!");
            return false;
        }      
    }
    
    return true;
};