#pragma once

#include <cstddef>
#include <cstdint>

template <typename T, size_t size >
class CommandFifo
{

public:
    CommandFifo();
    ~CommandFifo();

    //Interface
    bool push(const T& value);
    bool pop(T& value);
    size_t lenght();
    void flush();
    bool isfull();

private:
    uint32_t readPtr;
    uint32_t writePtr;
    T data[size];
};

template<typename T, size_t size>
inline CommandFifo<T, size>::CommandFifo()
{
    readPtr = 0;
    writePtr = 0;
}

template<typename T, size_t size>
inline CommandFifo<T, size>::~CommandFifo()
{
}

template<typename T, size_t size>
inline bool CommandFifo<T, size>::push(const T& value)
{
    if ((writePtr - readPtr) < size)
    {
        data[writePtr % size] = value;
        writePtr++;
        return true;
    }
    
    return false;
}

template<typename T, size_t size>
inline bool CommandFifo<T, size>::pop(T& value)
{
    if (readPtr < writePtr)
    {
        value = data[readPtr % size];
        readPtr++;

        // reset pointers to avoid overflow
        if (readPtr > size)
        {
            writePtr = writePtr % size;
            readPtr = readPtr % size;
        }
        return true;
    }
    return false;
}

template<typename T, size_t size>
inline size_t CommandFifo<T, size>::lenght()
{
    return (writePtr - readPtr);
}

template<typename T, size_t size>
inline void CommandFifo<T, size>::flush()
{
    writePtr = 0;
    readPtr = 0;
}

template<typename T, size_t size>
inline bool CommandFifo<T, size>::isfull()
{
    return (writePtr - readPtr) == size;
}

