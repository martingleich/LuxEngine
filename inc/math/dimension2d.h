#ifndef INCLUDED_DIMENSION2D_H
#define INCLUDED_DIMENSION2D_H
#include "math/vector2.h"

namespace lux
{
namespace math
{

//! Specifies a 2 dimensional size
template <typename T>
class dimension2d
{
public:
	//! Width of the dimension
	T width;
	//! Height of the dimension
	T height;

	//! default constructor for empty dimension
	dimension2d() : width(0), height(0)
	{
	}
	//! Constructor from width and height
	dimension2d(const T& width, const T& height) : width(width), height(height)
	{
	}
	//! Constructor from vector2
	dimension2d(const vector2<T>& v) : width(v.x), height(v.y)
	{
	}

	template <typename UType>
	explicit dimension2d(const dimension2d<UType>& other) : width((T)other.width), height((T)other.height)
	{
	}

	//! Assingment
	template <typename UType>
	dimension2d<T>& operator=(const dimension2d<UType>& other)
	{
		width = (T)other.width;
		height = (T)other.height;

		return *this;
	}

	//! Equality
	bool operator==(const dimension2d<T>& other) const
	{
		return (width == other.width) && (height == other.height);
	}

	//! Unquality
	bool operator!=(const dimension2d<T>& other) const
	{
		return !(*this == other);
	}

	//! Set the dimension
	dimension2d<T>& Set(const T& w, const T& h)
	{
		width = w;
		height = h;

		return *this;
	}

	//! scale dimension
	dimension2d<T>& operator/=(T s)
	{
		width /= s;
		height /= s;

		return *this;
	}

	//! scale dimension
	dimension2d<T> operator/(T s) const
	{
		return dimension2d<T>(width / s, height / s);
	}

	//! scale dimension
	dimension2d<T>& operator*=(T s)
	{
		width *= s;
		height *= s;

		return *this;
	}

	//! scale dimension
	dimension2d<T> operator*(T s) const
	{
		return dimension2d<T>(width*s, height*s);
	}

	//! The area of the dimension
	/**
	\return The area of the dimension
	*/
	T GetArea() const
	{
		return width * height;
	}

	//! Get a vector to the center of the area.
	vector2<T> GetCenter() const
	{
		return vector2<T>(width/2, height/2);
	}

	//! Linear interpolation betwenn this dimension and another
	/**
	\param other The dimension to interpolate with
	\param d The interpolation param between 0 and 1
	\return The interpolated dimension
	*/
	dimension2d<T> GetInterpolated(const dimension2d<T>& other, float d) const
	{
		float inv = (1.0f - d);
		return dimension2d<T>((T)(other.width*inv + width*d), (T)(other.height*inv + height*d));
	}

	//! Is the dimension a square
	/**
	\return Is the width and the height the same value
	*/
	bool IsSquare() const
	{
		return width == height;
	}

	//! Get the average size
	/**
	\return The average lenght of an edge.
	*/
	T GetAvgEdge() const
	{
		return (width+height)/2;
	}

	//! Is the dimension empty
	/**
	\return True if the dimension is empty
	*/
	bool IsEmpty() const
	{
		return math::Equal(width, 0) && math::Equal(height, 0);
	}

	//! The Aspect of the dimension
	/**
	The aspect euqals width/height
	\return The aspect of the dimension
	*/
	float GetAspect() const
	{
		return (float)width / height;
	}

	//! Fits this dimension into another
	/**
	\param other The dimension to test
	\return Does this dimension fist
	*/
	bool DoesFitInto(const dimension2d<T>& other) const
	{
		return (width <= other.width && height <= other.height);
	}

	//! Create a constrained dimension
	/**
	Creates a new dimension, from this one limited by constraints
	\param powerOfTwo The size of the result must be powers of two.
	\param square The result must be a square.
	\param larger The result should be larger than the current dimension,
		otherwise it is smaller.
	\param maxValue The result can't be bigger than this value,
		this value must fullfil the condition.
	\return The nearest dimension under the given constraints
	*/
	dimension2d<T> GetConstrained(
		bool powerOfTwo = true,
		bool square = false,
		bool larger = true,
		const dimension2d<T> maxValue = 0) const
	{
		T i = 1;
		T j = 1;
		if(powerOfTwo) {
			while(i < width)
				i *= 2;
			if(!larger && i != 1 && i != width)
				i /= 2;
			while(j < height)
				j *= 2;
			if(!larger && j != 1 && j != height)
				j /= 2;
		} else {
			i = width;
			j = height;
		}

		if(square) {
			if((larger && (i > j)) || (!larger && (i < j)))
				j = i;
			else
				i = j;
		}

		if(maxValue.width > 0 && i > maxValue.width)
			i = maxValue.width;

		if(maxValue.height > 0 && j > maxValue.height)
			j = maxValue.height;

		return dimension2d(i, j);
	}
};

///\cond INTERNAL
template <typename T>
dimension2d<T> operator/(T s, const dimension2d<T>& d)
{
	return d / s;
}

template <typename T>
dimension2d<T> operator*(T s, const dimension2d<T>& d)
{
	return d*s;
}
///\endcond

//! Dimension with float precision
typedef dimension2d<float> dimension2df;

//! Dimensino with unsigned int precision
typedef dimension2d<u32> dimension2du;

}
}

#endif // INCLUDED_DIMENSION2D_H



