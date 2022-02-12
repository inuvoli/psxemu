#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace lite
{
    template <typename T, size_t size >
    class fifo
    {

    public:
        fifo();
        ~fifo();

        //Interface
        bool push(const T& value);
        bool push(const std::vector<T>& value);
        bool pop(T& value);
        size_t lenght();
        void flush();
        bool isfull();
        bool isempty();

    private:
        uint32_t readPtr;
        uint32_t writePtr;
        T data[size];
    };

    template<typename T, size_t size>
    inline fifo<T, size>::fifo()
    {
        readPtr = 0;
        writePtr = 0;
    };

    template<typename T, size_t size>
    inline fifo<T, size>::~fifo()
    {
        //Nothing to do
    };

    template<typename T, size_t size>
    inline bool fifo<T, size>::push(const T& value)
    {
        if ((writePtr - readPtr) < size)
        {
            data[writePtr % size] = value;
            writePtr++;
            return true;
        };
        
        return false;
    };

    template<typename T, size_t size>
    inline bool fifo<T, size>::push(const std::vector<T>& value)
    {
        //check if there's enought space
        int32_t availableSpace = size - writePtr + readPtr;
        
        if (value.size() > availableSpace)
            return false;

        for (auto & e : value)
        {
            data[writePtr % size] = e;
            writePtr++;
        };
        
        return true;
    };

    template<typename T, size_t size>
    inline bool fifo<T, size>::pop(T& value)
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
            };
            return true;
        };
        return false;
    };

    template<typename T, size_t size>
    inline size_t fifo<T, size>::lenght()
    {
        return (writePtr - readPtr);
    };

    template<typename T, size_t size>
    inline void fifo<T, size>::flush()
    {
        writePtr = 0;
        readPtr = 0;
    };

    template<typename T, size_t size>
    inline bool fifo<T, size>::isfull()
    {
        return (writePtr - readPtr) == size;
    };

    template<typename T, size_t size>
    inline bool fifo<T, size>::isempty()
    {
        return (writePtr - readPtr) == 0;
    };
};


