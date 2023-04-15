#pragma once

namespace lite
{
    namespace
    {
        template <size_t LastBit>
        struct MinimumTypeHelper
        {
            typedef
                typename std::conditional<LastBit == 0, void,
                typename std::conditional<LastBit <= 8, uint8_t,
                typename std::conditional<LastBit <= 16, uint16_t,
                typename std::conditional<LastBit <= 32, uint32_t,
                typename std::conditional<LastBit <= 64, uint64_t,
                void>::type>::type>::type>::type>::type type;
        };

    }

    template <size_t Index, size_t Bits = 1>
    class bitfield
    {
    private:
        enum { Mask = (1u << Bits) - 1u };
        typedef typename MinimumTypeHelper<Index + Bits>::type T;
        T value_;

    public:
        template <class T2>
        bitfield& operator=(T2 value)
        {
            value_ = (value_ & ~(Mask << Index)) | ((value & Mask) << Index);
            return *this;
        }

        operator T() const { return (value_ >> Index) & Mask; }
        explicit operator bool() const { return value_ & (Mask << Index); }
        bitfield& operator++() { return *this = *this + 1; }
        T operator++(int) { T r = *this; ++* this; return r; }
        bitfield& operator--() { return *this = *this - 1; }
        T operator--(int) { T r = *this; --* this; return r; }
    };

    template <size_t Index>
    class bitfield<Index, 1>
    {
    private:
        enum { Bits = 1, Mask = 0x01 };
        typedef typename MinimumTypeHelper<Index + Bits>::type T;
        T value_;

    public:
        bitfield& operator=(bool value) {
            value_ = (value_ & ~(Mask << Index)) | (value << Index);
            return *this;
        }

        explicit operator bool() const { return value_ & (Mask << Index); }    
    };
}

