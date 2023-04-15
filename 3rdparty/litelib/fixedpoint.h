#pragma once

//-------------------------------------------------------------------------------------
// Generic Template Class for Fixed Point Matemathics
//-------------------------------------------------------------------------------------
namespace lite
{
    template<typename B, uint8_t I, uint8_t F = std::numeric_limits<B>::digits - I>
    class fixpnt
    {
        //Check Template Parameters at compile time
        static_assert(std::signed_integral<B>, "Fixed Point base type must be a Signed Integral");
		static_assert((I + F) <= std::numeric_limits<B>::digits, "Integer and Fractional part exceed base type capacity");

        //Grant access to private member _value to different size template
        friend class lite::fixpnt<B, I, F>;
        friend class std::numeric_limits<lite::fixpnt<B, I, F>>;

    public:
        //-----------------------------------------------------------------------------------------------
        // Class Constructors
        //-----------------------------------------------------------------------------------------------
    
        //Default Constructor;
        fixpnt() = default;
        	    
        //Constructor from integral type
        template<typename T>
        fixpnt(T value) : value_((B)value << F) {};
        
        //Constructor from boolean type
        fixpnt(bool value) : value_((B)(value * pow2<F>::value)) {};

        //Constructor from float type
        fixpnt(float value) : value_((B)(value * pow2<F>::value + (value >= 0 ? .5 : -.5))) {};
        
        //Constructor from double type
        fixpnt(double value) : value_((B)(value * pow2<F>::value + (value >= 0 ? .5 : -.5))) {};
        
        //Constructor from long double type
        fixpnt(long double value) : value_((B)(value * pow2<F>::value + (value >= 0 ? .5 : -.5))) {};

        //Constructor from fixed point type of the same size
        fixpnt(fixpnt<B, I, F> const& rhs) : value_(rhs.value_) {};
	    
        //Constructor from fixed point type of different size and precision
        template<typename B2, uint8_t I2, uint8_t F2>
	    fixpnt(fixpnt<B2, I2, F2> const& rhs)
        {    
            typedef std::conditional<(std::numeric_limits<B>::digits > std::numeric_limits<B2>::digits), B, B2>::type TypeResult;
            TypeResult tmp;
           
            tmp = rhs.value_;

            if (F2-F > 0)
                tmp >>= (F2-F);
            if (F-F2 > 0)
                tmp <<= (F-F2);

            value_ = static_cast<B>(tmp);
        }

        //Copy Operator beetwen fixed point of any size and precision
        template<typename B2, uint8_t I2, uint8_t F2>
        lite::fixpnt<B, I, F> & operator =(lite::fixpnt<B2, I2, F2> const& rhs)
        {
            //Create a Fixed Point of type <I, F> from a <I2, F2> type
            lite::fixpnt<B, I, F> temp(rhs);
            std::swap(value_, temp.value_);
            return *this;
        }

        //Comparison Operators
        auto operator<=>(const lite::fixpnt<B, I, F>&) const = default;
        bool operator ==(const lite::fixpnt<B, I, F>&) const = default;

        //Unary Operators
        lite::fixpnt<B, I, F> operator -() const
        {
            lite::fixpnt<B, I, F> result;
            result.value_ = -value_;
            return result;
        }
        
        lite::fixpnt<B, I, F> & operator ++()
        {
            value_ += pow2<F>::value;
            return *this;
        }

        lite::fixpnt<B, I, F> & operator --()
        {
            value_ -= pow2<F>::value;
            return *this;
        }

        lite::fixpnt<B, I, F> & operator +=(lite::fixpnt<B, I, F> const& summand)
        {
            value_ += summand.value_;
            return *this;
        }

        lite::fixpnt<B, I, F> & operator -=(lite::fixpnt<B, I, F> const& diminuend)
        {
            value_ -= diminuend.value_;
            return *this;
        }

        // Multiplication Operator
        lite::fixpnt<B, I, F> & operator *=(lite::fixpnt<B, I, F> const& factor)
        {
            value_ = (static_cast< typename lite::fixpnt<B, I, F>::template promote_type<B>::type>(value_) * factor.value_) >> F;
            return *this;
        }

        /// Division Operator
        lite::fixpnt<B, I, F> & operator /=(lite::fixpnt<B, I, F> const& divisor)
        {
            value_ = (static_cast<typename lite::fixpnt<B, I, F>::template promote_type<B>::type> (value_) << F) / divisor.value_;
            return *this;
        }

        friend lite::fixpnt<B, I, F> operator+ (lite::fixpnt<B, I, F> lhs, lite::fixpnt<B, I, F> const& rhs)
        {
            lhs += rhs;
            return lhs;
        }

        /// Shift right Operator.
        lite::fixpnt<B, I, F> & operator >>=(size_t shift)
        {
            value_ >>= shift;
            return *this;
        }

        /// Shift left Operator.
        lite::fixpnt<B, I, F> & operator <<=(size_t shift)
        {
            value_ <<= shift;
            return *this;
        }

        operator char() const
        {
            return (char)(value_ >> F);	
        }
        
        operator signed char() const
        {
            return (signed char)(value_ >> F);	
        }
        
        operator unsigned char() const
        {
            return (unsigned char)(value_ >> F);	
        }

        operator short() const
        {
            return (short)(value_ >> F);	
        }

        operator unsigned short() const
        {
            return (unsigned short)(value_ >> F);	
        }

        operator int() const
        {
            return (int)(value_ >> F);	
        }

        operator unsigned int() const
        {
            return (unsigned int)(value_ >> F);	
        }

        operator long() const
        {
            return (long)(value_ >> F);	
        }

        operator unsigned long() const
        {
            return (unsigned long)(value_ >> F);	
        }

        operator long long() const
        {
            return (long long)(value_ >> F);	
        }

        operator unsigned long long() const
        {
            return (unsigned long long)(value_ >> F);	
        }

        operator bool() const
        {
            return (bool)value_;	
        }

        operator float() const
        {
            return (float)value_ / pow2<F>::value;	
        }

        operator double() const
        {
            return (double)value_ / pow2<F>::value;	
        }

        operator long double() const
        {
            return (long double)value_ / pow2<F>::value;	
        }
    
        //-----------------------------------------------------------------------------------------------
        // Class Public Methods
        //-----------------------------------------------------------------------------------------------

        //Load value_ directly with raw data;
        void raw(B value) {value_ = value;};

	private:
        //-----------------------------------------------------------------------------------------------
        // Class Internal Helper Strict and Methods
        //-----------------------------------------------------------------------------------------------
        //Compile Time Power of two
        template<int P, typename T = void>
        struct pow2
        {
            static const long long value = 2 * pow2<P-1,T>::value;
        };

        template <typename P>
        struct pow2<0, P>
        {
            static const long long value = 1;
        };

		struct Error_promote_type_not_specialized_for_this_type {};

		template<typename T, typename U=void>
		struct promote_type
		{
			#ifdef _MSC_VER
			typedef Error_promote_type_not_specialized_for_this_type type;
			#endif // #ifdef _MSC_VER
		};

		template<typename U>
		struct promote_type<signed char, U>
		{
			typedef signed short type;
		};

		template<typename U>
		struct promote_type<unsigned char, U>
		{
			typedef unsigned short type;
		};

		template<typename U>
		struct promote_type<signed short, U>
		{
			typedef signed int type;
		};

		template<typename U>
		struct promote_type<unsigned short, U> 
		{
			typedef unsigned int type;
		};

		template<typename U>
		struct promote_type<signed int, U> 
		{
			typedef signed long long type;
		};

		template<typename U>
		struct promote_type<unsigned int, U> 
		{
			typedef unsigned long long type;
		};

    //-----------------------------------------------------------------------------------------------
    // Class Members
    //-----------------------------------------------------------------------------------------------
    public:
        //Fized Value sizes
        static const unsigned char integer_bit_count = I; 
	    static const unsigned char fractional_bit_count = F;

    private:
        //Fixed value data store
        B value_;
    };
};
