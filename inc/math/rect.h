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
	T left;    //!< The left edge of the rect
	T bottom; //!< The bottom edge of the rect

	T right; //!< The right edge of the rect            
	T top; //!< The top edge of the rect

	//! The empty rect located at 0, 0
	static const rect<T> EMPTY;

	//! default Constructor empty rect at 0,0
	rect() : left(0), bottom(0), right(0), top(0)
	{
	}

	//! Construct from two corners
	rect(T left, T top, T right, T bottom) : left(left), bottom(bottom), right(right), top(top)
	{
	}

	//! Construct from two corners
	rect(const vector2<T>& min, const vector2<T>& max) : left(min.x), bottom(min.y), right(max.x), top(Max.y)
	{
	}

	//! Copyconstruct
	rect(const rect& other) :
		left(other.left),
		bottom(other.bottom),
		right(other.right),
		top(other.top)
	{
	}

	//! Assignment
	rect& operator=(const rect& other)
	{
		left = other.left;
		bottom = other.bottom;
		right = other.right;
		top = other.top;

		return *this;
	}


	//! Equality
	bool operator==(const rect<T>& other) const
	{
		return (left == other.left && bottom == other.bottom && top == other.top && right == other.right);
	}

	//! Unequality
	bool operator!=(const rect<T>& other) const
	{
		return !(*this == other);
	}

	void Set(T left, T top, T right, T bottom)
	{
		left = left;
		bottom = bottom;
		right = right;
		top = top;
	}

	//! Return width of rect
	/**
	\return The width of the rect
	*/
	T GetWidth() const
	{
		return right - left;
	}

	//! Return height of rect
	/**
	\return The height of the rect
	*/
	T GetHeight() const
	{
		return bottom - top;
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
		return (right == left) || (top == bottom);
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

		if(left > right) {
			t = left;
			left = right;
			right = t;
		}
		if(bottom > top) {
			t = bottom;
			bottom = top;
			top = t;
		}
	}

	//! The lower left corner.
	vector2<T> Min() const
	{
		return vector2<T>(left, bottom);
	}

	//! The upper right corner
	vector2<T> Max() const
	{
		return vector2<T>(right, top);
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
		if(left < other.left)
			left = other.left;
		if(bottom > other.bottom)
			bottom = other.bottom;
		if(right > other.right)
			right = other.right;
		if(top < other.top)
			top = other.top;
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
