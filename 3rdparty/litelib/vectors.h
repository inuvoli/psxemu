#pragma once

//-------------------------------------------------------------------------------------
// Generic Template Class for 2, 3 and 4 element Vectors
//-------------------------------------------------------------------------------------
namespace lite
{
	template <typename T>
	struct vec2t
	{
		T	x;
		T	y;

		vec2t& operator= (const vec2t& t)
		{
			x = t.x;
			y = t.y;
			return *this;
		}
	};

	template <typename T>
	struct vec3t
	{
		T	x;
		T	y;
		T	z;

		vec3t& operator= (const vec3t& t)
		{
			x = t.x;
			y = t.y;
			z = t.z;
			return *this;
		}
	};

	template <typename T>
	struct vec4t
	{
		T	x1;
		T	y1;
		T	x2;
		T	y2;

		vec4t& operator= (const vec4t& t)
		{
			x1 = t.x1;
			y1 = t.y1;
			x2 = t.x2;
			y2 = t.y2;
			return *this;
		}
	};
}
