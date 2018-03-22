#ifndef INCLUDED_DIMENSION2D_H
#define INCLUDED_DIMENSION2D_H
#include "math/Vector2.h"

namespace lux
{
namespace math
{

//! Specifies a 2 dimensional size
template <typename T>
class Dimension2
{
public:
	//! Width of the dimension
	T width;
	//! Height of the dimension
	T height;

	//! default constructor for empty dimension
	Dimension2() : width(0), height(0)
	{
	}
	//! Constructor from width and height
	Dimension2(const T& width, const T& height) : width(width), height(height)
	{
	}
	//! Constructor from vector2
	Dimension2(const Vector2<T>& v) : width(v.x), height(v.y)
	{
	}

	static Dimension2 Square(T size)
	{
		return Dimension2(size, size);
	}

	template <typename UType>
	explicit Dimension2(const Dimension2<UType>& other) : width((T)other.width), height((T)other.height)
	{
	}

	//! Assingment
	template <typename UType>
	Dimension2<T>& operator=(const Dimension2<UType>& other)
	{
		width = (T)other.width;
		height = (T)other.height;

		return *this;
	}

	//! Equality
	bool operator==(const Dimension2<T>& other) const
	{
		return (width == other.width) && (height == other.height);
	}

	//! Unquality
	bool operator!=(const Dimension2<T>& other) const
	{
		return !(*this == other);
	}

	//! Set the dimension
	Dimension2<T>& Set(const T& w, const T& h)
	{
		width = w;
		height = h;

		return *this;
	}

	//! scale dimension
	Dimension2<T>& operator/=(T s)
	{
		width /= s;
		height /= s;

		return *this;
	}

	//! scale dimension
	Dimension2<T> operator/(T s) const
	{
		return Dimension2<T>(width / s, height / s);
	}

	//! scale dimension
	Dimension2<T>& operator*=(T s)
	{
		width *= s;
		height *= s;

		return *this;
	}

	//! scale dimension
	Dimension2<T> operator*(T s) const
	{
		return Dimension2<T>(width*s, height*s);
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
	Vector2<T> GetCenter() const
	{
		return Vector2<T>(width / 2, height / 2);
	}

	//! Linear interpolation betwenn this dimension and another
	/**
	\param other The dimension to interpolate with
	\param d The interpolation param between 0 and 1
	\return The interpolated dimension
	*/
	Dimension2<T> GetInterpolated(const Dimension2<T>& other, float d) const
	{
		float inv = (1.0f - d);
		return Dimension2<T>((T)(other.width*inv + width*d), (T)(other.height*inv + height*d));
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
		return (width + height) / 2;
	}

	//! Get the length of the diagonal
	T GetDiagonal() const
	{
		return std::sqrt(width*width+height*height);
	}

	//! Is the dimension empty
	/**
	\return True if the dimension is empty
	*/
	bool IsEmpty() const
	{
		return math::IsEqual(width, 0) && math::IsEqual(height, 0);
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
	bool DoesFitInto(const Dimension2<T>& other) const
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
	Dimension2<T> GetConstrained(
		bool powerOfTwo = true,
		bool square = false,
		bool larger = true,
		const Dimension2<T> maxValue = 0) const
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

		return Dimension2(i, j);
	}
};

///\cond INTERNAL
template <typename T>
Dimension2<T> operator/(T s, const Dimension2<T>& d)
{
	return d / s;
}

template <typename T>
Dimension2<T> operator*(T s, const Dimension2<T>& d)
{
	return d*s;
}
///\endcond

//! Dimension with float precision
typedef Dimension2<float> Dimension2F;

//! Dimension with unsigned int precision
typedef Dimension2<int> Dimension2I;

} // namespace math
} // namespace lux

#endif // INCLUDED_DIMENSION2D_H



