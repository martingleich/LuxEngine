#ifndef INCLUDED_LUX_MATH_PIXEL_TRANSFORMATION_H
#define INCLUDED_LUX_MATH_PIXEL_TRANSFORMATION_H
#include "math/Vector2.h"

namespace lux
{
namespace math
{

/*
A flip rotation is equal to rotations in 90° steps and mirrorings along the x and y axis in 2D.
*/
class PixelFlipRot
{
public:
	//! Create the identity flip rotation.
	PixelFlipRot() :
		m_S(false),
		m_N1(false),
		m_N2(false)
	{
	}
	//! Add a rotation in 90 degree steps counter clockwise.
	PixelFlipRot& Rotate(int i)
	{
		// Positive modulo 4
		i = (i%4+4)%4;
		switch(i) {
		case 1: return Append(PixelFlipRot(true, true, false));
		case 2: return Append(PixelFlipRot(false, true, true));
		case 3: return Append(PixelFlipRot(true, false, true));
		}
		return *this;
	}
	//! Add a horizontal flip.
	PixelFlipRot& FlipH() { return Append(PixelFlipRot(false, true, false)); }
	//! Add a vertical flip.
	PixelFlipRot& FlipV() { return Append(PixelFlipRot(false, false, true)); }
	//! Add another flip rotation.
	PixelFlipRot& Append(PixelFlipRot other)
	{
		return (*this = GetAppended(other));
	}
	//! Invert this flip rotation.
	PixelFlipRot& Invert()
	{
		return (*this = GetInverted());
	}
	//! Get the result of appending another flip rotation to this one.
	PixelFlipRot GetAppended(PixelFlipRot other) const
	{
		// Algorithm follows directly from the machine describtion.
		return PixelFlipRot(
			m_S != other.m_S,
			(other.m_S ? m_N2 : m_N1) != other.m_N1,
			(other.m_S ? m_N1 : m_N2) != other.m_N2);
	}
	//! Get the inverse flip rotation to this one.
	PixelFlipRot GetInverted() const
	{
		// Algorithm follows directly from the machine describtion.
		return PixelFlipRot(
			m_S,
			m_S ? m_N2 : m_N1,
			m_S ? m_N1 : m_N2);
	}
	//! Apply this flip rotation to a vector.
	Vector2F Apply(Vector2F v) const
	{
		return Vector2F(
			m_S ? (m_N1 ? -v.y : v.y) : (m_N1 ? -v.x : v.x),
			m_S ? (m_N2 ? -v.x : v.x) : (m_N2 ? -v.y : v.y));
	}

private:
	PixelFlipRot(bool s, bool n1, bool n2) :
		m_S(s),
		m_N1(n1),
		m_N2(n2)
	{
	}

private:
	// The parameters describe a machine for manipulation coordinates.
	// First if m_S is set the coordinates will be swapped, otherwise just passed trough.
	// If m_N1 is set the first resulting coordinate is negated.
	// If m_N2 is set the second resulting coordinate is negated.
	// Each set of parameters equals exactly one of the 8 possible flip rotations.
	bool m_S;
	bool m_N1;
	bool m_N2;
};

/*
A pixel transform describes transformation on a grid.
It only allows rotations in 90° steps and mirrorings along x and y axes,
and translations.
*/
class PixelTransform
{
public:
	//! Create a identity transformation.
	PixelTransform() = default;

	//! Add a horizontal flip.
	PixelTransform& FlipH()
	{
		m_FlipRot.FlipH();
		return *this;
	}
	//! Add a vertical flip.
	PixelTransform& FlipV()
	{
		m_FlipRot.FlipV();
		return *this;
	}
	//! Add a rotation in 90 degree steps counter clockwise.
	PixelTransform& Rotate(int steps)
	{
		m_FlipRot.Rotate(steps);
		return *this;
	}
	//! Add a translation	
	PixelTransform& Translate(float x, float y)
	{
		return Translate({x,y});
	}
	//! Add a translation	
	PixelTransform& Translate(Vector2F v)
	{
		m_Offset += v;
		return *this;
	}
	//! Add any other pixel transform.
	PixelTransform& Append(Vector2F off, PixelFlipRot flipRot)
	{
		m_Offset += flipRot.Apply(off);
		m_FlipRot.Append(flipRot);
		return *this;
	}
	//! Add any other pixel transform.
	PixelTransform& Append(PixelTransform other)
	{
		return Append(other.m_Offset, other.m_FlipRot);
	}

	PixelTransform GetAppended(PixelTransform other) const
	{
		return PixelTransform(*this).Append(other);
	}

	//! Apply the transformation to a vector.
	Vector2F Apply(const Vector2F& v) const
	{
		return m_FlipRot.Apply(v) + m_Offset;
	}
	//! Apply the transformation without translation to a vector.
	Vector2F ApplyDir(const Vector2F& v) const
	{
		return m_FlipRot.Apply(v);
	}

private:
	Vector2F m_Offset;
	PixelFlipRot m_FlipRot;
};

}
}

#endif // #ifndef INCLUDED_LUX_MATH_PIXEL_TRANSFORMATION_H
