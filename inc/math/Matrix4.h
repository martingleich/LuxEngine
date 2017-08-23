#ifndef INCLUDED_MATRIX4_H
#define INCLUDED_MATRIX4_H
#include "math/Vector2.h"
#include "Plane.h"
#include "math/Quaternion.h"

namespace lux
{
namespace math
{

//! Represent a 4x4 matrix
class LUX_API Matrix4
{
private:
	float m[4][4];

public:
	static const Matrix4 IDENTITY; //!< The identity matrix
	static const Matrix4 ZERO; //!< The null matrix

	//! Constructor for a identity matrix
	Matrix4();

	//! Copyconstructor
	/**
	\param m The other matrix
	*/
	Matrix4(const Matrix4& m);

	//! Full constructor
	Matrix4(float c11, float c12, float c13, float c14,
		float c21, float c22, float c23, float c24,
		float c31, float c32, float c33, float c34,
		float c41, float c42, float c43, float c44)
	{
		m[0][0] = c11; m[0][1] = c12; m[0][2] = c13; m[0][3] = c14;
		m[1][0] = c21; m[1][1] = c22; m[1][2] = c23; m[1][3] = c24;
		m[2][0] = c31; m[2][1] = c32; m[2][2] = c33; m[2][3] = c34;
		m[3][0] = c41; m[3][1] = c42; m[3][2] = c43; m[3][3] = c44;
	}

	//! Is this matrix the identity matrix
	/**
	\return True if this matrix is ident
	*/
	bool IsIdent() const;

	//! Get the transposed of this matrix
	Matrix4 GetTransposed() const;

	//! Get the inverted of a transformation matrix.
	/**
	This method assumes the right most collum is (0,0,0,1).
	\param [out] result If not null, true if the matrix was succesfully inverted otherwise false
	\return The inverted matrix, or identity if not possible
	*/
	Matrix4 GetTransformInverted(bool* result = nullptr) const;

	//! Transform this matrix to a 3x3 matrix
	/**
	The 3x3 matrix is located in the upper left corner of the 4x4
	For transfomations of twodimension objects
	\param [out] out Here the 3x3 matrix is written
	*/
	Matrix4 Get3x3() const;

	//! Returns the determinant of a transformation matrix
	/**
	This method assumes the right most collum is (0,0,0,1).
	\return The determinat of the matrix
	*/
	float GetTransformDet() const;

	//! Make this matrix to a identity matrix
	/**
	\return Selfreference
	*/
	Matrix4& MakeIdent();

	//! Invert this transformation matrix
	/**
	If the inversion failed, the matrix is set to identity
	\param [out] result If not null, true if the matrix was succesfully inverted otherwise false
	\return Selfreference
	*/
	Matrix4& InvertTransform(bool* result=nullptr);
	bool SetByInvertTransform(const Matrix4& m);

	//! Transpose this matrix
	/**
	\return Selfreference
	*/
	Matrix4& Transpose();

	//! Transforms a vector with this matrix
	/**
	The case w!=1, is ignored, meaning no projection is done
	\param v The vector to transform
	\return The transformed vector
	*/
	inline Vector3F TransformVector(const Vector3F& v) const
	{
		Vector3F out;

		out.x = v.x*m[0][0] + v.y*m[1][0] + v.z*m[2][0] + m[3][0];
		out.y = v.x*m[0][1] + v.y*m[1][1] + v.z*m[2][1] + m[3][1];
		out.z = v.x*m[0][2] + v.y*m[1][2] + v.z*m[2][2] + m[3][2];

		return out;
	}

	//! Apply only the rotation of this matrix to a vector
	/**
	\param v The vector to rotate
	\return The rotated vector
	*/
	inline Vector3F RotateVector(const Vector3F& v) const
	{
		Vector3F out;
		out.x = v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2];
		out.y = v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2];
		out.z = v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2];

		return out;
	}

	//! Transforms a array of vector with this matrix
	/**
	in and out are allowed to be overlapping, with the right alignment
	\param in The array to transform
	\param [out] out The array where the transformend vectors are written
	\param count The number of vectors in the array
	*/
	inline void TransformVectorArray(const Vector3F in[], Vector3F out[], int count) const
	{
		for(int i = 0; i < count; ++i, ++in, ++out) {
			const float x = in->x*m[0][0] + in->y*m[1][0] + in->z*m[2][0] + m[3][0];
			const float y = in->x*m[0][1] + in->y*m[1][1] + in->z*m[2][1] + m[3][1];
			const float z = in->x*m[0][2] + in->y*m[1][2] + in->z*m[2][2] + m[3][2];
			out->x = x;
			out->y = y;
			out->z = z;
		}
	}

	//! Transforms a vector returnig the new w coordinate
	/**
	\param in The vector to transform
	\param [out] out Here the transformed vector is written
	*/
	inline void TransformVectorW(const Vector3F& in, float out[4]) const
	{
		out[0] = in.x*m[0][0] + in.y*m[1][0] + in.z*m[2][0] + m[3][0];
		out[1] = in.x*m[0][1] + in.y*m[1][1] + in.z*m[2][1] + m[3][1];
		out[2] = in.x*m[0][2] + in.y*m[1][2] + in.z*m[2][2] + m[3][2];
		out[3] = in.x*m[0][3] + in.y*m[1][3] + in.z*m[2][3] + m[3][3];
	}

	//! Transforms a plane with the matrix
	/**
	\param p The plane to transform
	\return The transformed plane
	*/
	inline PlaneF TransformPlane(const PlaneF& p) const
	{
		PlaneF out;
		out.normal.x = p.normal.x * m[0][0] + p.normal.y * m[1][0] + p.normal.z * m[2][0];
		out.normal.y = p.normal.x * m[0][1] + p.normal.y * m[1][1] + p.normal.z * m[2][1];
		out.normal.z = p.normal.x * m[0][2] + p.normal.y * m[1][2] + p.normal.z * m[2][2];
		out.d = p.d - p.normal.x * m[0][3] + p.normal.y * m[1][3] + p.normal.z * m[2][3];
		return out;
	}

	//! Set the translation done by this matrix
	/**
	The Tranlation is set in the order Scalation->Rotation->Translation
	\param Trans The new used translation
	\return Selfreference
	*/
	Matrix4& SetTranslation(const Vector3F& Trans);

	//! Creates a Translationmatrix and combine it with this matrix
	/**
	\param Trans The Translation to add
	\return Selfreference
	*/
	Matrix4& AddTranslation(const Vector3F& Trans);

	//! The current translation done by this matrix
	/**
	\return The Translation done by this matrix
	*/
	Vector3F GetTranslation() const;

	//! Set the scaling done by this matrix
	/**
	The scalation is set in the order Scalation->Rotation->Translation.
	Don't use this method if already a rotation is contained in the matrix
	\param scale The new used scale
	\return Selfreference
	*/
	Matrix4& SetScale(const Vector3F& scale);

	//! Creates a scalematrix and combine it with this matrix
	/**
	\param scale The new used scale
	\return Selfreference
	*/
	Matrix4& AddScale(const Vector3F& scale);

	//! The current scale done by this matrix
	/**
	\return The scale done by this matrix
	*/
	Vector3F GetScale() const;

	//! Set the rotation done by this matrix
	/**
	Performs a euler rotation in order x y z.
	\param x The x angle.
	\param y The y angle.
	\param z The z angle.
	\return Selfreference
	*/
	Matrix4& SetRotationEuler(AngleF x, AngleF y, AngleF z);

	//! Set the rotation done by this matrix
	/**
	The rotations are done in the order XYZ
	\param f The rotation about the x axis.
	\return Selfreference
	*/
	Matrix4& SetRotationX(AngleF f);

	//! Set the rotation done by this matrix
	/**
	The rotations are done in the order XYZ
	\param f The rotation about the y axis
	\return Selfreference
	*/
	Matrix4& SetRotationY(AngleF f);

	//! Set the rotation done by this matrix
	/**
	The rotations are done in the order XYZ
	\param f The rotation about the z axis.
	\return Selfreference
	*/
	Matrix4& SetRotationZ(AngleF f);

	//! Add a rotation about the x axis
	/**
	\param f The rotation about the x axis.
	\return Selfreference
	*/
	Matrix4& AddRotationX(AngleF f);

	//! Add a rotation about the y axis
	/**
	\param f The rotation about the y axis.
	\return Selfreference
	*/
	Matrix4& AddRotationY(AngleF f);

	//! Add a rotation about the z axis
	/**
	\param f The rotation about the z axis.
	\return Selfreference
	*/
	Matrix4& AddRotationZ(AngleF f);

	//! Add a euler rotation to this matrix
	/**
	\param Rot The rotation vector in Eulerangle(XYZ) and Rad
	\return Selfreference
	*/
	Matrix4& AddRotation(AngleF x, AngleF y, AngleF z);

	//! The rotation done by this matrix
	/**
	Calculates the rotation in Eulerangle(XYZ) and Degree
	\return The rotation done by this matrix
	*/
	Vector3F GetRotationDeg() const;

	//! Set this matrix to the product of two other matrices
	/**
	\param a The first factor
	\param b The second factor
	\return Selfreference
	*/
	Matrix4& SetByProduct(const Matrix4& a, const Matrix4& b);

	//! Make a worldmatrix
	/**
	Make this matrix perform first a scale then a Rotation then a Translation
	\param scale The scale for all three axes to perform
	\param Rot The rotation to perforn in Eulerangles(XYZ) and Rad
	\param Trans The translation to perform
	\return Selfreference
	*/
	Matrix4& BuildWorld(const Vector3F& scale,
		const Vector3F& Rot,
		const Vector3F& Trans);

	//! Make a worldmatrix
	/**
	Make this matrix perform first a scale then a Rotation then a Translation
	\param scale The scale for all three axes to perform
	\param Orient The rotation to perform
	\param Trans The translation to perform
	\return Selfreference
	*/
	Matrix4& BuildWorld(const Vector3F& scale,
		const QuaternionF& Orient,
		const Vector3F& Trans);

	//! Make a perspective projection matrix(hyperbolic Z-transform)
	/**
	\param FOVY The vertical field of vision in rad
	\param Aspect screenwidth/screenheight
	\param NearPlane The near clipping plane
	\param FarPlane The far clipping plane
	\return Selfreference
	*/
	Matrix4& BuildProjection_Persp(float FOVY,
		float Aspect,
		float NearPlane,
		float FarPlane);

	//! Make a orthogonale projection matrix
	/**
	\param XMax The biggest x-coordinate to show(the screen goes from -XMax to XMax)
	\param Aspect screenwidth/screenheight
	\param NearPlane The near clipping plane
	\param FarPlane The far clipping plane
	\return Selfreference
	*/
	Matrix4& BuildProjection_Ortho(float XMax,
		float Aspect,
		float NearPlane,
		float FarPlane);


	//! Make a camera matrix
	/**
	\param position The position of the camera
	\param Dir The direction the camera is looking, must not be normalized
	\param Up The upside of the camera, must not be normalized
	\return Selfreference
	*/
	Matrix4& BuildCamera(const Vector3F& position,
		const Vector3F& Dir,
		const Vector3F& Up = Vector3F::UNIT_Y);

	//! Make a camera matrix
	/**
	\param position The position of the camera
	\param LookAt The point the camera is watching
	\param Up The upside of the camera, must not be normalized
	\return Selfreference
	*/
	Matrix4& BuildCameraLookAt(const Vector3F& position,
		const Vector3F& LookAt,
		const Vector3F& Up = Vector3F::UNIT_Y);

	//! Access a element of the matrix
	/**
	\param Row The row to access[0-3]
	\param Col The collum to access[0-3]
	\return The element at the specified position
	*/
	float& operator() (int Row, int Col)
	{
		lxAssert(Row >= 0 && Row < 4 && Col >= 0 && Col < 4);
		return m[Row][Col];
	}

	//! Access a element of the matrix
	/**
	\param Row The row to access[0-3]
	\param Col The collum to access[0-3]
	\return The element at the specified position
	*/
	const float& operator() (int Row, int Col) const
	{
		lxAssert(Row >= 0 && Row < 4 && Col >= 0 && Col < 4);
		return m[Row][Col];
	}

	//! Access a element of the matrix
	/**
	\param elem The Elementid, beginning by zero rowwise from top-left to bottom-right
	\return The element at the specified position
	*/
	float& operator[] (int elem)
	{
		lxAssert(elem >= 0 && elem < 16);
		return ((float*)m)[elem];
	}

	//! Access a element of the matrix
	/**
	\param elem The Elementid, beginning by zero rowwise from top-left to bottom-right
	\return The element at the specified position
	*/
	float operator[] (int elem) const
	{
		lxAssert(elem >= 0 && elem < 16);
		return ((const float*)m)[elem];
	}

	//! A pointer to the matrix data in row-major order.
	float* DataRowMajor()
	{
		return (float*)m;
	}

	//! A pointer to the matrix data in row-major order.
	const float* DataRowMajor() const
	{
		return (const float*)m;
	}

	//! Addition
	Matrix4 operator+ (const Matrix4& other) const;
	//! Short addition
	Matrix4& operator+=(const Matrix4& other);
	//! Subtraction
	Matrix4 operator- (const Matrix4& other) const;
	//! Short subtraction
	Matrix4& operator-=(const Matrix4& other);
	//! Scalar multiplication
	Matrix4 operator* (float f) const;
	//! Short scalar multiplication
	Matrix4& operator*=(float f);
	//! Multiplication
	Matrix4 operator* (const Matrix4& other) const;
	//! Shortmuliplication
	Matrix4& operator*=(const Matrix4& other);

	//! Assignment
	Matrix4& operator= (const Matrix4& other);

	//! Equality
	bool operator==(const Matrix4& other) const;
	//! Unequality
	bool operator!=(const Matrix4& other) const;
};

///\cond INTERNAL
inline Matrix4 operator*(float f, const Matrix4& m)
{
	return m*f;
}
///\endcond


inline bool IsEqual(const Matrix4& a, const Matrix4& b, float tolerance = math::Constants<float>::rounding_error())
{
	for(int i = 0; i < 16; ++i) {
		if(!math::IsEqual(a[i], b[i], tolerance))
			return false;
	}

	return true;
}

} // namespace math

namespace core
{
namespace Types
{
LUX_API Type Matrix();
}

template<> inline Type GetTypeInfo<math::Matrix4>() { return Types::Matrix(); };
} // namespace core

} // namespace lux

#endif

