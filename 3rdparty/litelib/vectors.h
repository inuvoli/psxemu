#pragma once
//-------------------------------------------------------------------------------------
// Generic Template Class for 2, 3 and 4 element Vectors
//-------------------------------------------------------------------------------------
namespace lite
{
	template <typename T>
	struct vec2t
	{
		union {T x, u, c1;};
		union {T y, v, c2;};

		vec2t() = default;
		vec2t(T x_, T y_) : x(x_), y(y_) {};

		vec2t operator- () const
        {
			vec2t<T> res;
			res.x = -x;
			res.y = -y;
            return res;
        }
		
		vec2t& operator= (vec2t const& t)
		{
			x = t.x;
			y = t.y;
			return *this;
		}

		vec2t& operator+= (vec2t const& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		vec2t& operator-= (vec2t const& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		friend vec2t operator+ (vec2t lhs, vec2t const& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		friend vec2t operator- (vec2t lhs, vec2t const& rhs)
		{
			lhs -= rhs;
			return lhs;
		}
	};

	template <typename T>
	struct vec3t
	{
		union {T x, r, c1;};
		union {T y, g, c2;};
		union {T z, b, c3;};

		vec3t() = default;
		vec3t(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {};
		
		vec3t operator- () const
        {
			vec3t<T> res;
			res.x = -x;
			res.y = -y;
			res.z = -z;
            return res;
        }

		vec3t& operator= (vec3t const& t)
		{
			x = t.x;
			y = t.y;
			z = t.z;
			return *this;
		}

		vec3t& operator+= (vec3t const& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		vec3t& operator-= (vec3t const& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		friend vec3t operator+ (vec3t lhs, vec3t const& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		friend vec3t operator- (vec3t lhs, vec3t const& rhs)
		{
			lhs -= rhs;
			return lhs;
		}
	};

	template <typename T>
	struct vec4t
	{
		union {T x, r, c1, x1;};
		union {T y, g, c2, y1;};
		union {T z, b, c3, x2;};
		union {T w, a, c4, y2;};

		vec4t() = default;
		vec4t(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {};

		vec4t operator- () const
        {
			vec4t<T> res;
			res.x = -x;
			res.y = -y;
			res.z = -z;
			res.w = -w;
            return res;
        }

		vec4t& operator= (vec4t const& t)
		{
			x = t.x;
			y = t.y;
			z = t.z;
			w = t.w;
			return *this;
		}

		vec4t& operator+= (vec4t const& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		vec4t& operator-= (vec4t const& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}

		friend vec4t operator+ (vec4t lhs, vec4t const& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		friend vec4t operator- (vec4t lhs, vec4t const& rhs)
		{
			lhs -= rhs;
			return lhs;
		}
	};
}

//-------------------------------------------------------------------------------------
// Vectors Short Definitions
//-------------------------------------------------------------------------------------
namespace lite
{
	typedef vec2t<int> ivec2;
	typedef vec2t<int8_t> i8vec2;
	typedef vec2t<int16_t> i16vec2;
	typedef vec2t<int32_t> i32vec2;
	typedef vec2t<int64_t> i64vec2;
	typedef vec2t<unsigned int> uivec2;
	typedef vec2t<uint8_t> ui8vec2;
	typedef vec2t<uint16_t> ui16vec2;
	typedef vec2t<uint32_t> ui32vec2;
	typedef vec2t<uint64_t> ui64vec2;

	typedef vec3t<int> ivec3;
	typedef vec3t<int8_t> i8vec3;
	typedef vec3t<int16_t> i16vec3;
	typedef vec3t<int32_t> i32vec3;
	typedef vec3t<int64_t> i64vec3;
	typedef vec3t<unsigned int> uivec3;
	typedef vec3t<uint8_t> ui8vec3;
	typedef vec3t<uint16_t> ui16vec3;
	typedef vec3t<uint32_t> ui32vec3;
	typedef vec3t<uint64_t> ui64vec3;

	typedef vec4t<int> ivec4;
	typedef vec4t<int8_t> i8vec4;
	typedef vec4t<int16_t> i16vec4;
	typedef vec4t<int32_t> i32vec4;
	typedef vec4t<int64_t> i64vec4;
	typedef vec4t<unsigned int> uivec4;
	typedef vec4t<uint8_t> ui8vec4;
	typedef vec4t<uint16_t> ui16vec4;
	typedef vec4t<uint32_t> ui32vec4;
	typedef vec4t<uint64_t> ui64vec4;
}
