#ifndef INCLUDED_MATRIX4_H
#define INCLUDED_MATRIX4_H
#include "math/vector2.h"
#include "plane3D.h"
#include "math/quaternion.h"

namespace lux
{
namespace math
{

//! Represent a 4x4 matrix
class LUX_API matrix4
{
public:
	//! How should a matrix be created
	enum EMatrix4Constructor
	{
		M4C_NOTHING,    //!< Do nothing at creation
		M4C_IDENT,        //!< Set to identiy
		M4C_COPY,        //!< Copy another matrix
		M4C_INV,        //!< Set to inversion of another matrix
		M4C_TRANSP,        //!< Set to transposion of another matrix
		M4C_INV_TRANSP    //!< Set to transposed inversion of another matrix
	};

private:
	float m[4][4];

public:
	static const matrix4 IDENTITY;    //!< The identity matrix
	static const matrix4 ZERO;    //!< The null matrix

	//! Constructor for a new matrix
	/**
	\param Constructor How should the new matrix created, only M4C_NOTHING and M4C_IDENT
	*/
	matrix4::matrix4(EMatrix4Constructor Constructor = M4C_IDENT);

	//! Copyconstructor
	/**
	\param m The other matrix
	\param Constructor How should the new matrix created
	*/
	matrix4::matrix4(const matrix4& m, EMatrix4Constructor Constructor = M4C_COPY);

	//! Full constructor
	matrix4::matrix4(float c11, float c12, float c13, float c14,
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
	bool    IsIdent() const;
	//! Transpose this matrix
	/**
	\param [out] out Here the transposed matrix is written
	*/
	void    GetTransposed(matrix4& out) const;
	//! Invert this matrix
	/**
	\param [out] out Here the inverted matrix is written
	\return false if the inversion is imposible
	*/
	bool    GetInverted(matrix4& out) const;
	//! transform this matrix to a 3x3 matrix(the last row and collum are empty)
	/**
	For transfomations of twodimension objects
	\param [out] out Here the 3x3 matrix is written
	*/
	void    Get3x3(matrix4& out) const;
	//! Returns the determinant of the matrix
	/**
	Calculates only the determinant of the 3x4 matrix
	\return The determinat of the matrix
	*/
	float   GetDet() const;

	//! Make this matrix to a identity matrix
	/**
	\return Selfreference
	*/
	matrix4& MakeIdent();
	//! Invert this matrix
	/**
	\return Selfreference
	*/
	matrix4& Invert();
	//! Transpose this matrix
	/**
	\return Selfreference
	*/
	matrix4& Transpose();

	//! Transforms a vector with this matrix
	/**
	The case w!=1, is ignored, meaning no projection is done
	\param v The vector to transform
	\return The transformed vector
	*/
	inline vector3f TransformVector(const vector3f& v) const
	{
		vector3f out;

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
	inline vector3f RotateVector(const vector3f& v) const
	{
		vector3f out;
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
	inline void TransformVectorArray(const vector3f in[], vector3f out[], int count) const
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
	inline void TransformVectorW(const vector3f& in, float out[4]) const
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
	inline plane3df TransformPlane(const plane3df& p) const
	{
		plane3df out;
		out.Normal.x = p.Normal.x * m[0][0] + p.Normal.y * m[1][0] + p.Normal.z * m[2][0];
		out.Normal.y = p.Normal.x * m[0][1] + p.Normal.y * m[1][1] + p.Normal.z * m[2][1];
		out.Normal.z = p.Normal.x * m[0][2] + p.Normal.y * m[1][2] + p.Normal.z * m[2][2];
		out.D = p.D - p.Normal.x * m[0][3] + p.Normal.y * m[1][3] + p.Normal.z * m[2][3];
		return out;
	}

	//! Set the translation done by this matrix
	/**
	The Tranlation is set in the order Scalation->Rotation->Translation
	\param Trans The new used translation
	\return Selfreference
	*/
	inline matrix4& SetTranslation(const vector3f& Trans);

	//! Creates a Translationmatrix and combine it with this matrix
	/**
	\param Trans The Translation to add
	\return Selfreference
	*/
	inline matrix4& AddTranslation(const vector3f& Trans);

	//! The current translation done by this matrix
	/**
	\return The Translation done by this matrix
	*/
	inline vector3f GetTranslation() const;

	//! Set the scaling done by this matrix
	/**
	The scalation is set in the order Scalation->Rotation->Translation
	\param scale The new used scale
	\return Selfreference
	*/
	inline matrix4& SetScale(const vector3f& scale);

	//! Creates a scalematrix and combine it with this matrix
	/**
	\param scale The new used scale
	\return Selfreference
	*/
	inline matrix4& AddScale(const vector3f& scale);

	//! The current scale done by this matrix
	/**
	\return The scale done by this matrix
	*/
	inline vector3f GetScale() const;

	//! Set the rotation done by this matrix
	/**
	Performs a euler rotation in order x y z.
	\param x The x angle.
	\param y The y angle.
	\param z The z angle.
	\return Selfreference
	*/
	inline matrix4& SetRotationEuler(anglef x, anglef y, anglef z);

	//! Set the rotation done by this matrix
	/**
	The rotations are done in the order XYZ
	\param f The rotation about the x axis.
	\return Selfreference
	*/
	inline matrix4& SetRotationX(anglef f);

	//! Set the rotation done by this matrix
	/**
	The rotations are done in the order XYZ
	\param f The rotation about the y axis
	\return Selfreference
	*/
	inline matrix4& SetRotationY(anglef f);

	//! Set the rotation done by this matrix
	/**
	The rotations are done in the order XYZ
	\param f The rotation about the z axis.
	\return Selfreference
	*/
	inline matrix4& SetRotationZ(anglef f);

	//! Add a rotation about the x axis
	/**
	\param f The rotation about the x axis.
	\return Selfreference
	*/
	inline matrix4& AddRotationX(anglef f);

	//! Add a rotation about the y axis
	/**
	\param f The rotation about the y axis.
	\return Selfreference
	*/
	inline matrix4& AddRotationY(anglef f);

	//! Add a rotation about the z axis
	/**
	\param f The rotation about the z axis.
	\return Selfreference
	*/
	inline matrix4& AddRotationZ(anglef f);

	//! Add a euler rotation to this matrix
	/**
	\param Rot The rotation vector in Eulerangle(XYZ) and Rad
	\return Selfreference
	*/
	inline matrix4& AddRotation(anglef x, anglef y, anglef z);

	//! The rotation done by this matrix
	/**
	Calculates the rotation in Eulerangle(XYZ) and Degree
	\return The rotation done by this matrix
	*/
	inline vector3f GetRotationDeg() const;

	//! Set this matrix to the product of two other matrices
	/**
	\param a The first factor
	\param b The second factor
	\return Selfreference
	*/
	inline matrix4& SetByProduct(const matrix4& a, const matrix4& b);

	//! Make a worldmatrix
	/**
	Make this matrix perform first a scale then a Rotation then a Translation
	\param scale The scale for all three axes to perform
	\param Rot The rotation to perforn in Eulerangles(XYZ) and Rad
	\param Trans The translation to perform
	\return Selfreference
	*/
	matrix4& BuildWorld(const vector3f& scale,
		const vector3f& Rot,
		const vector3f& Trans);

	//! Make a worldmatrix
	/**
	Make this matrix perform first a scale then a Rotation then a Translation
	\param scale The scale for all three axes to perform
	\param Orient The rotation to perform
	\param Trans The translation to perform
	\return Selfreference
	*/
	matrix4& BuildWorld(const vector3f& scale,
		const quaternionf& Orient,
		const vector3f& Trans);

	//! Make a perspective projection matrix(hyperbolic Z-transform)
	/**
	\param FOV The field of vision in rad
	\param Aspect screenwidth/screenheight
	\param NearPlane The near clipping plane
	\param FarPlane The far clipping plane
	\return Selfreference
	*/
	matrix4& BuildProjection_Persp(float FOV,
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
	matrix4& BuildProjection_Ortho(float XMax,
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
	matrix4& BuildCamera(const vector3f& position,
		const vector3f& Dir,
		const vector3f& Up = vector3f::UNIT_Y);

	//! Make a camera matrix
	/**
	\param position The position of the camera
	\param LookAt The point the camera is watching
	\param Up The upside of the camera, must not be normalized
	\return Selfreference
	*/
	matrix4& BuildCameraLookAt(const vector3f& position,
		const vector3f& LookAt,
		const vector3f& Up = vector3f::UNIT_Y);

	//! Access a element of the matrix
	/**
	\param Row The row to access[0-3]
	\param Col The collum to access[0-3]
	\return The element at the specified position
	*/
	float& operator() (int Row, int Col)
	{
		assert(Row >= 0 && Row < 4 && Col >= 0 && Col < 4);
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
		assert(Row >= 0 && Row < 4 && Col >= 0 && Col < 4);
		return m[Row][Col];
	}

	//! Access a element of the matrix
	/**
	\param elem The Elementid, beginning by zero rowwise from top-left to bottom-right
	\return The element at the specified position
	*/
	float& operator[] (int elem)
	{
		assert(elem >= 0 && elem < 16);
		return ((float*)m)[elem];
	}

	//! Access a element of the matrix
	/**
	\param elem The Elementid, beginning by zero rowwise from top-left to bottom-right
	\return The element at the specified position
	*/
	float operator[] (int elem) const
	{
		assert(elem >= 0 && elem < 16);
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
	inline matrix4  operator+ (const matrix4& other) const;
	//! Short addition
	inline matrix4& operator+=(const matrix4& other);
	//! Subtraction
	inline matrix4  operator- (const matrix4& other) const;
	//! Short subtraction
	inline matrix4& operator-=(const matrix4& other);
	//! Scalar multiplication
	inline matrix4  operator* (float f) const;
	//! Short scalar multiplication
	inline matrix4& operator*=(float f);
	//! Multiplication
	inline matrix4  operator* (const matrix4& other) const;
	//! Shortmuliplication
	inline matrix4& operator*=(const matrix4& other);

	//! Assignment
	inline matrix4& operator= (const matrix4& other);

	//! Equality
	inline bool operator==(const matrix4& other) const;
	//! Unequality
	inline bool operator!=(const matrix4& other) const;

	//! Equality with tolerance
	bool Equal(const matrix4& other, float tolerance = math::Constants<float>::rounding_error()) const;
};

///\cond INTERNAL
inline matrix4 operator* (float f, const matrix4& m)
{
	return m*f;
}
///\endcond
}

template<> inline core::Type core::GetTypeInfo<math::matrix4>() { return core::Type::Matrix; };

}    


#endif

