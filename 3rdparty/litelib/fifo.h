#pragma once

//-------------------------------------------------------------------------------------
// Generic Class for FIFO Queue
//-------------------------------------------------------------------------------------

namespace lite
{
    template <typename T, size_t size >
    class fifo
    {

    public:
        fifo();
        ~fifo();

        //Interface
        bool    push(const T& value);                  //push a record on the fifo
        bool    push(const std::vector<T>& value);     //push a vector on the fifo
        bool    pop(T& value);                         //pop a record from the fifo
        size_t  lenght();                            //return fifo lenght in number od records
        void    flush();                               //empty the fifo
        bool    isfull();                              //check if the fifo is full
        bool    isempty();                             //check if the fifo is empty
        T       head();                                //return the first record on the fifo without pop it from the queue

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

    template<typename T, size_t size>
    inline T fifo<T, size>::head()
    {
        return data[readPtr % size];
    };
};


