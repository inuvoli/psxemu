#pragma once

//-------------------------------------------------------------------------------------
// Generic Template Class for 2x2, 3x3 and 4x4 element Matrix
//-------------------------------------------------------------------------------------
namespace lite
{
    template <typename T>
	struct mat2t
	{
		vec2t<T> r1, r2;
		
		mat2t() = default;
		mat2t(vec2t<T> r1_, vec2t<T> r2_) : r1(r1_), r2(r2_) {};

		mat2t operator- () const
        {
			lite::mat2t<T> res;
			res.r1 = -r1;
			res.r2 = -r2;
            return res;
        }
		
		mat2t& operator= (mat2t const& t)
		{
			r1 = t.r1;
			r2 = t.r2;
			return *this;
		}

		mat2t& operator+= (mat2t const& rhs)
		{
			r1 += rhs.r1;
			r2 += rhs.r2;
			return *this;
		}

		mat2t& operator-= (mat2t const& rhs)
		{
			r1 -= rhs.r1;
			r2 -= rhs.r2;
			return *this;
		}

		friend mat2t operator+ (mat2t lhs, mat2t const& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		friend mat2t operator- (mat2t lhs, mat2t const& rhs)
		{
			lhs -= rhs;
			return lhs;
		}
	};

	template <typename T>
	struct mat3t
	{
		vec3t<T> r1, r2, r3;
		
		mat3t() = default;
		mat3t(vec3t<T> r1_, vec3t<T> r2_, vec3t<T> r3_) : r1(r1_), r2(r2_), r3(r3_) {};

		mat3t operator- () const
        {
			lite::mat3t<T> res;
			res.r1 = -r1;
			res.r2 = -r2;
			res.r3 = -r3;
            return res;
        }
		
		mat3t& operator= (mat3t const& t)
		{
			r1 = t.r1;
			r2 = t.r2;
			r3 = t.r3;
			return *this;
		}

		mat3t& operator+= (mat3t const& rhs)
		{
			r1 += rhs.r1;
			r2 += rhs.r2;
			r3 += rhs.r3;
			return *this;
		}

		mat3t& operator-= (mat3t const& rhs)
		{
			r1 -= rhs.r1;
			r2 -= rhs.r2;
			r3 -= rhs.r3;
			return *this;
		}

		friend mat3t operator+ (mat3t lhs, mat3t const& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		friend mat3t operator- (mat3t lhs, mat3t const& rhs)
		{
			lhs -= rhs;
			return lhs;
		}
	};

	template <typename T>
	struct mat4t
	{
		vec4t<T> r1, r2, r3, r4;
		
		mat4t() = default;
		mat4t(vec3t<T> r1_, vec3t<T> r2_, vec3t<T> r3_, vec3t<T> r4_) : r1(r1_), r2(r2_), r3(r3_), r4(r4_) {};

		mat4t operator- () const
        {
			lite::mat4t<T> res;
			res.r1 = -r1;
			res.r2 = -r2;
			res.r3 = -r3;
			res.r4 = -r4;
            return res;
        }
		
		mat4t& operator= (mat4t const& t)
		{
			r1 = t.r1;
			r2 = t.r2;
			r3 = t.r3;
			r4 = t.r4;
			return *this;
		}

		mat4t& operator+= (mat4t const& rhs)
		{
			r1 += rhs.r1;
			r2 += rhs.r2;
			r3 += rhs.r3;
			r4 += rhs.r4;
			return *this;
		}

		mat4t& operator-= (mat4t const& rhs)
		{
			r1 -= rhs.r1;
			r2 -= rhs.r2;
			r3 -= rhs.r3;
			r4 -= rhs.r4;
			return *this;
		}

		friend mat4t operator+ (mat4t lhs, mat4t const& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		friend mat4t operator- (mat4t lhs, mat4t const& rhs)
		{
			lhs -= rhs;
			return lhs;
		}
	};
}