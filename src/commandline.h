#pragma once

#include <string>
#include <algorithm>

class commandline
{
public:
    static commandline& instance()
    {
        static commandline *instance = new commandline();
        return *instance;
    }

    bool parse(int argc, char* argv[]);
    std::string getBiosFileName() { return biosFilename; };
    std::string getBinFileName() { return binFilename; };
    std::string getExeFileName() { return exeFilename; };

private:
    commandline() {}

    bool checkCommand(char** begin, char** end, const std::string &cmd);
    char* getStringValue(char** begin, char** end, const std::string &cmd);
    int getIntValue(char** begin, char** end, const std::string &cmd);

    std::string        biosFilename;
    std::string        exeFilename;
    std::string        binFilename;


};