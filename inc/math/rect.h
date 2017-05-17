#ifndef INCLUDED_RECT_H
#define INCLUDED_RECT_H
#include "dimension2d.h"

namespace lux
{
namespace math
{

//! A two-dimensional rectangle    
/**
The x axis runs from left to right
The y axis runs from bottom to top
*/
template <typename T>
class rect
{
public:
	T Left;    //!< The left edge of the rect
	T Bottom; //!< The bottom edge of the rect

	T Right; //!< The right edge of the rect            
	T Top; //!< The top edge of the rect

	//! The empty rect located at 0, 0
	static const rect<T> EMPTY;

	//! default Constructor empty rect at 0,0
	rect() : Left(0), Bottom(0), Right(0), Top(0)
	{
	}

	//! Construct from two corners
	rect(T left, T top, T right, T bottom) : Left(left), Bottom(bottom), Right(right), Top(top)
	{
	}

	//! Construct from two corners
	rect(const vector2<T>& min, const vector2<T>& max) : Left(min.x), Bottom(min.y), Right(max.x), Top(Max.y)
	{
	}

	//! Copyconstruct
	rect(const rect& other) :
		Left(other.Left),
		Bottom(other.Bottom),
		Right(other.Right),
		Top(other.Top)
	{
	}

	//! Assignment
	rect& operator=(const rect& other)
	{
		Left = other.Left;
		Bottom = other.Bottom;
		Right = other.Right;
		Top = other.Top;

		return *this;
	}


	//! Equality
	bool operator==(const rect<T>& other) const
	{
		return (Left == other.Left && Bottom == other.Bottom && Top == other.Top && Right == other.Right);
	}

	//! Unequality
	bool operator!=(const rect<T>& other) const
	{
		return !(*this == other);
	}

	void Set(T left, T top, T right, T bottom)
	{
		Left = left;
		Bottom = bottom;
		Right = right;
		Top = top;
	}

	//! Return width of rect
	/**
	\return The width of the rect
	*/
	T GetWidth() const
	{
		return Right - Left;
	}

	//! Return height of rect
	/**
	\return The height of the rect
	*/
	T GetHeight() const
	{
		return Bottom - Top;
	}

	//! Return area of rect
	/**
	\return The area of the rect
	*/
	T GetArea() const
	{
		return GetWidth() * GetHeight();
	}

	//! Is the rect empty
	/**
	\return True if the rect is empty, otherwise false
	*/
	bool IsEmpty() const
	{
		return (Right == Left) || (Top == Bottom);
	}

	//! Get the dimension of rect
	/**
	\return The dimension of the rect
	*/
	dimension2d<T> GetSize() const
	{
		return dimension2d<T>(GetWidth(), GetHeight());
	}

	//! Repair rect if invalid
	/**
	Invalid means min-edge is bigger than max-edge
	*/
	void Repair()
	{
		T t;

		if(Left > Right) {
			t = Left;
			Left = Right;
			Right = t;
		}
		if(Bottom > Top) {
			t = Bottom;
			Bottom = Top;
			Top = t;
		}
	}

	//! The lower left corner.
	vector2<T> Min() const
	{
		return vector2<T>(Left, Bottom);
	}

	//! The upper right corner
	vector2<T> Max() const
	{
		return vector2<T>(Right, Top);
	}

	//! Get center point of rect
	/**
	\return The center of the rect
	*/
	vector2<T> GetCenter() const
	{
		return (Min() + Max()) / (T)2;
	}

	//! Fits this rect into another
	/**
	Shrinks this rect to be fully inside another rect
	\param other The rect to fit in
	\return Selfreference
	*/
	rect<T>& FitInto(const rect<T>& other)
	{
		if(Left < other.Left)
			Left = other.Left;
		if(Bottom > other.Bottom)
			Bottom = other.Bottom;
		if(Right > other.Right)
			Right = other.Right;
		if(Top < other.Top)
			Top = other.Top;
		return *this;
	}
};

template <typename T>
const rect<T> rect<T>::EMPTY = rect<T>(0, 0, 0, 0);

//! Typedef for rect with float precision
typedef rect<float> rectf;
//! Typedef for rect with integer precision
typedef rect<s32>    recti;

} // !namespace math
} // !namespace lux

#endif // !INCLUDED_RECT2D_H
