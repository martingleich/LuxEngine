#ifndef INCLUDED_RECT_H
#define INCLUDED_RECT_H
#include "Dimension2.h"

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
class Rect
{
public:
	T left;    //!< The left edge of the rect
	T bottom; //!< The bottom edge of the rect

	T right; //!< The right edge of the rect            
	T top; //!< The top edge of the rect

	//! The empty rect located at 0, 0
	static const Rect<T> EMPTY;

	//! default Constructor empty rect at 0,0
	Rect() : left(0), bottom(0), right(0), top(0)
	{
	}

	//! Construct from two corners
	Rect(T left, T top, T right, T bottom) : left(left), bottom(bottom), right(right), top(top)
	{
	}

	//! Construct from two corners
	Rect(const Vector2<T>& min, const Vector2<T>& max) : left(min.x), bottom(min.y), right(max.x), top(Max.y)
	{
	}

	//! Copyconstruct
	Rect(const Rect& other) :
		left(other.left),
		bottom(other.bottom),
		right(other.right),
		top(other.top)
	{
	}

	//! Assignment
	Rect& operator=(const Rect& other)
	{
		left = other.left;
		bottom = other.bottom;
		right = other.right;
		top = other.top;

		return *this;
	}


	//! Equality
	bool operator==(const Rect<T>& other) const
	{
		return (left == other.left && bottom == other.bottom && top == other.top && right == other.right);
	}

	//! Unequality
	bool operator!=(const Rect<T>& other) const
	{
		return !(*this == other);
	}

	void Set(T _left, T _top, T _right, T _bottom)
	{
		left = _left;
		bottom = _bottom;
		right = _right;
		top = _top;
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
	Dimension2<T> GetSize() const
	{
		return Dimension2<T>(GetWidth(), GetHeight());
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

	//! Is the rectangle valie
	/**
	Invalid means min-edge is bigger than max-edge
	*/
	bool IsValid() const
	{
		return left <= right && top <= bottom;
	}

	//! The lower left corner.
	Vector2<T> Min() const
	{
		return Vector2<T>(left, bottom);
	}

	//! The upper right corner
	Vector2<T> Max() const
	{
		return Vector2<T>(right, top);
	}

	//! Get center point of rect
	/**
	\return The center of the rect
	*/
	Vector2<T> GetCenter() const
	{
		return (Min() + Max()) / (T)2;
	}

	//! Fits this rect into another
	/**
	Shrinks this rect to be fully inside another rect
	\param other The rect to fit in
	\return Selfreference
	*/
	Rect<T>& FitInto(const Rect<T>& other)
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

	//! Check if a point is inside the rect, including the edge
	bool IsInside(const Vector2<T>& point) const
	{
		if(point.x < left)
			return false;
		if(point.x > right)
			return false;
		if(point.y < top)
			return false;
		if(point.y > bottom)
			return false;
		return true;
	}
};

template <typename T>
const Rect<T> Rect<T>::EMPTY = Rect<T>(0, 0, 0, 0);

//! Typedef for rect with float precision
typedef Rect<float> RectF;
//! Typedef for rect with integer precision
typedef Rect<s32> RectI;
typedef Rect<u32> RectU;

} // !namespace math
} // !namespace lux

#endif // !INCLUDED_RECT2D_H
