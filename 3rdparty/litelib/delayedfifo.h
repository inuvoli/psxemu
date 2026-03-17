#pragma once

//-------------------------------------------------------------------------------------
// Generic Class for Delayed FIFO Queue
//-------------------------------------------------------------------------------------

namespace lite
{
    template <typename T, size_t size >
    class delayedfifo
    {

    public:
        delayedfifo();
        ~delayedfifo();

        //Interface
        bool    push(const T& _data, int _delay = 1);                                   //push a record on the fifo with a delay
        bool    push(const std::vector<T>& _data, const std::vector<int>& _delay);      //push a vector on the fifo with delays
        bool    pop(T& value);                                                          //pop a record from the fifo
        size_t  length();                                                               //return fifo length in number of records
        void    flush();                                                                //empty the fifo
        bool    isfull();                                                               //check if the fifo is full
        bool    isempty();                                                              //check if the fifo is empty
        T&      operator[](size_t index);                                               //access element by index
        const T& operator[](size_t index) const;                                        //const access element by index

    private:
        uint32_t readPtr;
        uint32_t writePtr;
        T data[size];
        int delay[size];
    };

    template<typename T, size_t size>
    inline delayedfifo<T, size>::delayedfifo()
    {
        readPtr = 0;
        writePtr = 0;
    };

    template<typename T, size_t size>
    inline delayedfifo<T, size>::~delayedfifo()
    {
        //Nothing to do
    };

    template<typename T, size_t size>
    inline bool delayedfifo<T, size>::push(const T& _data, int _delay)
    {
        if ((writePtr - readPtr) < size)
        {
            data[writePtr % size] = _data;
            delay[writePtr % size] = _delay;
            writePtr++;
            return true;
        };
        
        return false;
    };

    template<typename T, size_t size>
    inline bool delayedfifo<T, size>::push(const std::vector<T>& _data, const std::vector<int>& _delay)
    {
        //check if there's enought space
        int32_t availableSpace = size - writePtr + readPtr;
        
        if (_data.size() > availableSpace)
            return false;

        int startPtr;

		startPtr = writePtr;
        for (auto & e : _data)
        {
            data[startPtr % size] = e;
            startPtr++;
        };

		startPtr = writePtr;
		for (auto& d : _delay)
        {
            delay[startPtr % size] = d;
            startPtr++;
        };
        
		writePtr += _data.size();
        return true;
    };

    template<typename T, size_t size>
    inline bool delayedfifo<T, size>::pop(T& value)
    {
		if (delay[readPtr % size] > 0)
        {
            delay[readPtr % size]--;
            return false;
        }
        else
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
            }
            else
            {
                return false;
            }
        }
    };

    template<typename T, size_t size>
    inline size_t delayedfifo<T, size>::length()
    {
        return (writePtr - readPtr);
    };

    template<typename T, size_t size>
    inline void delayedfifo<T, size>::flush()
    {
        writePtr = 0;
        readPtr = 0;
    };

    template<typename T, size_t size>
    inline bool delayedfifo<T, size>::isfull()
    {
        return (writePtr - readPtr) == size;
    };

    template<typename T, size_t size>
    inline bool delayedfifo<T, size>::isempty()
    {
        return (writePtr - readPtr) == 0;
    };   
};


