#ifndef INCLUDED_TRANSFORMATION_H
#define INCLUDED_TRANSFORMATION_H
#include "math/Matrix4.h"
#include "math/Triangle3.h"
#include "core/lxFormat.h"

namespace lux
{
namespace math
{

//! Describes a Transformation in 3D, containg a translation, a Rotation, and a Scalation
/**
Transformation are applied in order scale->Rotate->Translate
*/
class Transformation
{
public:
	float                scale;              //!< The Scalation in all directions
	math::QuaternionF    orientation;        //!< A quaternion holding the orientation, user should ensure unit-length
	math::Vector3F       translation;        //!< The translation

public:
	LUX_API static const Transformation DEFAULT;    //! The default transformation

public:
	//! default Constructor
	/**
	No translation, or Rotation and at unit size
	*/
	Transformation() : scale(1.0f), orientation(0.0f, 0.0f, 0.0f, 1.0f), translation(0.0f, 0.0f, 0.0f)
	{
	}

	//! Constructor
	Transformation(const math::Vector3F& Trans,
		const math::QuaternionF& Orient = math::QuaternionF(0.0f, 0.0f, 0.0f, 1.0f),
		float _Scale = 1.0f) : scale(_Scale), orientation(Orient), translation(Trans)
	{
	}

	//! Make the Transformation, to reverse the old Transformation
	Transformation& Invert()
	{
		scale = 1.0f / scale;
		orientation.Conjungate();
		translation = -1.0f * scale * orientation.TransformInPlace(translation);
		return *this;
	}

	//! Make a Transformation, reversing this one
	/**
	\param out: Here the new Transformation is written
	*/
	Transformation& GetInverted(Transformation& out) const
	{
		out.scale = 1.0f / scale;
		out.orientation = orientation.GetConjungate();
		out.translation = -1.0f * out.scale * out.orientation.Transform(translation);
		return out;
	}

	//! Make a Transformation, reversing this one
	/**
	\return The inverted transform
	*/
	Transformation GetInverted() const
	{
		Transformation out;
		return GetInverted(out);
	}

	//! Transforms a single Point
	/**
	\param in: The point to transform
	\param out: Here the transformed point is written
	\return: The transformed point
	*/
	math::Vector3F& TransformPoint(const math::Vector3F& in,
		math::Vector3F& out) const
	{
		out = in * scale;
		orientation.TransformInPlace(out);
		out += translation;
		return out;
	}

	//! Transforms a single point
	/**
	\param in: The point to transform
	\return: The transformed point
	*/
	math::Vector3F TransformPoint(const math::Vector3F& in) const
	{
		math::Vector3F out;
		return TransformPoint(in, out);
	}

	//! Transforms a point with the inversed Transformation
	/**
	\param in: The point to transform
	\param out: Here the transformed point is written
	\return: The transformed point
	*/
	math::Vector3F& TransformInvPoint(const math::Vector3F& in,
		math::Vector3F& out) const
	{
		const float recScale = 1.0f / scale;
		out = in * recScale;
		orientation.TransformInPlaceInv(out);
		out -= recScale * orientation.TransformInv(translation);
		return out;
	}

	//! Transforms a point with the inversed Transformation
	/**
	\param in: The point to transform
	\return: The transformed point
	*/
	math::Vector3F TransformInvPoint(const math::Vector3F& in) const
	{
		math::Vector3F out;
		return TransformInvPoint(in, out);
	}

	//! Transforms a direction(only Rotation and scale)
	/**
	\param in: The direction to transform
	\param out: Here the transformed direction is written
	\return: The transformed direction
	*/
	math::Vector3F& TransformDir(const math::Vector3F& in,
		math::Vector3F& out) const
	{
		out = orientation.Transform(in * scale);
		return out;
	}

	//! Transforms a direction(only Rotation and scale)
	/**
	\param in: The direction to transform
	\return: The transformed direction
	*/
	math::Vector3F TransformDir(const math::Vector3F& in) const
	{
		math::Vector3F out;
		return TransformDir(in, out);
	}

	//! Transforms a direction(only Rotation and scale) with the inversed transform
	/**
	\param in: The direction to transform
	\param out: Here the transformed direction is written
	\return: The transformed direction
	*/
	math::Vector3F& TransformInvDir(const math::Vector3F& in,
		math::Vector3F& out) const
	{
		out = orientation.TransformInv(in) / scale;
		return out;
	}

	//! Transforms a direction(only Rotation and scale) with the inversed transform
	/**
	\param in: The direction to transform
	\return: The transformed direction
	*/
	math::Vector3F TransformInvDir(const math::Vector3F& in) const
	{
		math::Vector3F out;
		return TransformInvDir(in, out);
	}

	//! Make this Transformation as combination of this transform followed by another transform
	/**
	\param other: The transformation to add
	\return: The new Transformation
	*/
	Transformation& AddRight(const Transformation& other)
	{
		scale *= other.scale;
		orientation *= other.orientation;

		translation *= other.scale;
		other.orientation.TransformInPlace(translation);
		translation += other.translation;

		return *this;
	}

	//! Make this Transformation as combination of another transform followed by this transform
	/**
	\param other: The transformation to add
	\return: The new Transformation
	*/
	Transformation& AddLeft(const Transformation& other)
	{
		const math::Vector3F v = translation;

		translation = other.translation * scale;
		orientation.TransformInPlace(translation);
		translation += v;

		scale *= other.scale;
		orientation *= other.orientation;

		return *this;
	}

	//! Make a new Transformation as combination of this transform followed by another transform
	/**
	\param other: The transformation to add
	\return: The new Transformation
	*/
	Transformation CombineRight(const Transformation& other) const
	{
		Transformation out = *this;
		out.AddRight(other);
		return out;
	}

	//! Make a new Transformation as combination of another transform followed by this transform
	/**
	\param other: The transformation to add
	\return: The new Transformation
	*/
	Transformation CombineLeft(const Transformation& other) const
	{
		Transformation out = other;
		out.AddRight(*this);
		return out;
	}

	//! Assignment operator
	Transformation& operator=(const Transformation& other)
	{
		scale = other.scale;
		orientation = other.orientation;
		translation = other.translation;
		return *this;
	}

	//! Equality opererator
	bool operator==(const Transformation& other) const
	{
		return ((scale == other.scale) &&
			(translation == other.translation) &&
			(orientation == other.orientation));
	}

	//! Inequality operator
	bool operator!=(const Transformation& other) const
	{
		return !(*this == other);
	}

	//! Builds a matrix from this transform
	/**
	\param out: Here the new matrix is written
	*/
	void ToMatrix(math::Matrix4& out) const
	{
		const float xy = orientation.x * orientation.y;
		const float yy = orientation.y * orientation.y;
		const float xx = orientation.x * orientation.x;
		const float zw = orientation.z * orientation.w;
		const float zz = orientation.z * orientation.z;
		const float xz = orientation.x * orientation.z;
		const float yw = orientation.y * orientation.w;
		const float zy = orientation.z * orientation.y;
		const float xw = orientation.x * orientation.w;

		const float Scale2 = 2.0f * scale;

		out(0, 0) = scale * (1.0f - 2.0f * (yy + zz));
		out(0, 1) = Scale2 * (xy + zw);
		out(0, 2) = Scale2 * (xz - yw);
		out(0, 3) = 0.0f;

		out(1, 0) = Scale2 * (xy - zw);
		out(1, 1) = scale * (1.0f - 2.0f * (xx + zz));
		out(1, 2) = Scale2 * (zy + xw);
		out(1, 3) = 0.0f;

		out(2, 0) = Scale2 * (xz + yw);
		out(2, 1) = Scale2 * (zy - xw);
		out(2, 2) = scale * (1.0f - 2.0f * (xx + yy));
		out(2, 3) = 0.0f;

		out(3, 0) = translation.x;
		out(3, 1) = translation.y;
		out(3, 2) = translation.z;
		out(3, 3) = 1.0f;
	}

	math::Matrix4 ToMatrix() const
	{
		math::Matrix4 out;
		ToMatrix(out);

		return out;
	}

	//! Builds a inverted matrix from this transform
	/**
	\param out: Here the new matrix is written
	*/
	void ToMatrixInv(math::Matrix4& out) const
	{
		const float xy = orientation.x * orientation.y;
		const float yy = orientation.y * orientation.y;
		const float xx = orientation.x * orientation.x;
		const float zw = orientation.z * orientation.w;
		const float zz = orientation.z * orientation.z;
		const float xz = orientation.x * orientation.z;
		const float yw = orientation.y * orientation.w;
		const float zy = orientation.z * orientation.y;
		const float xw = orientation.x * orientation.w;

		const float recScale = 1.0f / this->scale;
		const float recScale2 = 2.0f * recScale;

		out(0, 0) = recScale * (1.0f - 2.0f * (yy + zz));
		out(0, 1) = recScale2 * (xy - zw);
		out(0, 2) = recScale2 * (xz + yw);
		out(0, 3) = 0.0f;

		out(1, 0) = recScale2 * (xy + zw);
		out(1, 1) = recScale * (1.0f - 2.0f * (xx + zz));
		out(1, 2) = recScale2 * (zy - xw);
		out(1, 3) = 0.0f;

		out(2, 0) = recScale2 * (xz - yw);
		out(2, 1) = recScale2 * (zy + xw);
		out(2, 2) = recScale * (1.0f - 2.0f * (xx + yy));
		out(2, 3) = 0.0f;

		const math::Vector3F InvTrans = -recScale * orientation.TransformInv(translation);

		out(3, 0) = InvTrans.x;
		out(3, 1) = InvTrans.y;
		out(3, 2) = InvTrans.z;
		out(3, 3) = 1.0f;
	}

	math::Matrix4 ToMatrixInv() const
	{
		math::Matrix4 out;
		ToMatrixInv(out);

		return out;
	}

	//! Can be specialized by a user to transform other classes
	template <typename T>
	T& TransformObject(const T& in, T& out) const
	{
		lxAssertNeverReach("TransformObject for this type not implemented.");
		return out;
	}

	//! Can be specialized by a user to transform other classes
	template <typename T>
	T TransformObject(const T& in) const
	{
		T out;
		TransformObject(in, out);
		return out;
	}
};

inline bool IsEqual(const Transformation& a, const Transformation& b, float tolerance = math::Constants<float>::rounding_error())
{
	if(math::IsZero(a.scale, tolerance) && math::IsZero(b.scale, tolerance))
		return true;

	return 
		math::IsEqual(a.scale, b.scale, tolerance) &&
		math::IsEqual(a.translation, b.translation, tolerance) &&
		(math::IsEqual(a.orientation, b.orientation, tolerance) || math::IsEqual(a.orientation, -b.orientation, tolerance));
}

template <>
inline math::Line3F& Transformation::TransformObject(const math::Line3F& in, math::Line3F& out) const
{
	this->TransformPoint(in.start, out.start);
	this->TransformPoint(in.end, out.end);

	return out;
}

template <>
inline math::PlaneF& Transformation::TransformObject(const math::PlaneF& in, math::PlaneF& out) const
{
	out.normal = orientation.Transform(in.normal);
	out.RecalculateD(this->TransformPoint(in.GetMemberPoint()));

	return out;
}

template <>
inline math::Triangle3F& Transformation::TransformObject(const math::Triangle3F& in, math::Triangle3F& out) const
{
	out.Set(TransformPoint(in.A), TransformPoint(in.B), TransformPoint(in.C));
	return out;
}

inline void conv_data(format::Context& ctx, const Transformation& t, format::Placeholder& placeholder)
{
	using namespace format;
	ConvertAddString(ctx, format::StringType::Ascii, "[transl=", 8);
	conv_data(ctx, t.translation, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " orient=", 8);
	conv_data(ctx, t.orientation, placeholder);
	ConvertAddString(ctx, format::StringType::Ascii, " scale=", 7);
	conv_data(ctx, t.scale, placeholder);

	ConvertAddString(ctx, format::StringType::Ascii, "]", 1);
}

} // !namespace math
} // !namespace lux

#endif // !INCLUDED_TRANSFORMATION_H
