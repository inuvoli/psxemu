#pragma once

//-------------------------------------------------------------------------------------
// Generic Class for Circular Buffer
//-------------------------------------------------------------------------------------

namespace lite
{
    template <typename T, size_t size>
    class circularbuffer
    {
        public:
            circularbuffer();
            ~circularbuffer();

            //Interface
            void write(T data);
            T read();
            T inspect(int pos);
            int lenght();

        private:
        T   _data[size];
        int _readPtr;
        int _writePtr;
        int _lenght;
    };

    template<typename T, size_t size>
    inline circularbuffer<T, size>::circularbuffer()
    {
        _readPtr = 0;
        _writePtr = 0;
        _lenght = 0;
    };

    template<typename T, size_t size>
    inline circularbuffer<T, size>::~circularbuffer()
    {
        //Nothing to do
    };

    template<typename T, size_t size>
    inline void circularbuffer<T, size>::write(T data)
    {
        _data[_writePtr % size] = data;
        _writePtr++;
        _lenght++;
        if (_lenght > size)
            _lenght = size;
        return;
    }

    template<typename T, size_t size>
    inline T circularbuffer<T, size>::read()
    {
        if (_lenght == 0)
            return 0;
        
        T tmp = _data[_readPtr % size];
        _readPtr++;
        _lenght--;

        return tmp;
    }

    template<typename T, size_t size>
    inline T circularbuffer<T, size>::inspect(int pos)
    {           
        return _data[(_writePtr - pos - 1) % size];
    }

    template<typename T, size_t size>
    inline int circularbuffer<T, size>::lenght()
    {
        return _lenght;
    }
};

