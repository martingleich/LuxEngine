#ifndef INCLUDED_S3DVERTEX_H
#define INCLUDED_S3DVERTEX_H
#include "math/vector2.h"
#include "math/vector3.h"
#include "video/Color.h"

namespace lux
{
namespace video
{

//! The standard 3d vertex
/**
\ref VertexFormat::STANDARD
*/
class Vertex3D
{
public:
	math::vector3f position; //!< The vertex position
	math::vector3f normal; //!< The vertex normal
	video::Color color; //!< The vertex color
	math::vector2f texture; //!< The vertex texturecoords

	//! Default constructor
	Vertex3D()
	{
	}

	//! Construct from single values
	Vertex3D(float Px, float Py, float Pz, Color _Color, float Nx, float Ny, float Nz, float Tx, float Ty) :
		position(Px, Py, Pz), normal(Nx, Ny, Nz), texture(Tx, Ty), color(_Color)
	{
	}

	//! Construct from combined values
	Vertex3D(const math::vector3f& _Pos, Color _Color, const math::vector3f& _Normal, const math::vector2f& _TCoords)
		: position(_Pos), normal(_Normal), texture(_TCoords), color(_Color)
	{
	}

	//! Equality
	bool operator==(const Vertex3D& other) const
	{
		return ((position == other.position) && (normal == other.normal) &&
			(color == other.color) && (texture == other.texture));
	}

	//! Unequality
	bool operator!=(const Vertex3D& other) const
	{
		return ((position != other.position) || (normal != other.normal) ||
			(color != other.color) || (texture != other.texture));
	}

	//! Smaller
	bool operator<(const Vertex3D& other) const
	{
		return ((position < other.position) ||
			((position == other.position) && (normal < other.normal)) ||
			((position == other.position) && (normal == other.normal) && (color < other.color)) ||
			((position == other.position) && (normal == other.normal) && (color == other.color) && (texture < other.texture)));
	}
};

//! A simple 2d vertex
/**
\ref EVT_2DVERTEX
*/
class Vertex2D
{
public:
	math::vector2f position; //!< The vertex position
	video::Color color; //!< The vertex color
	math::vector2f texture; //! The vertex texturecoords

	//! Default constructor
	Vertex2D()
	{
	}

	//! Construct from single values
	Vertex2D(float Px, float Py, Color _Color, float Tx, float Ty) :
		position(Px, Py), texture(Tx, Ty), color(_Color)
	{
	}

	//! Construct from combined values
	Vertex2D(float Px, float Py, Color _Color) :
		position(Px, Py), texture(0.0f, 0.0f), color(_Color)
	{
	}

	//! Full constructor
	Vertex2D(const math::vector2f& _Pos, Color _Color, const math::vector2f& _TCoords)
		: position(_Pos), texture(_TCoords), color(_Color)
	{
	}

	//! Equality
	bool operator==(const Vertex2D& other) const
	{
		return (color == other.color) && (texture == other.texture);
	}

	//! Unquality
	bool operator!=(const Vertex2D& other) const
	{
		return ((position != other.position) ||
			(color != other.color) || (texture != other.texture));
	}

	//! Smaller
	bool operator<(const Vertex2D& other) const
	{
		return ((position < other.position) ||
			((position == other.position) && (color < other.color)) ||
			((position == other.position) && (color == other.color) && (texture < other.texture)));
	}
};

//! Transformed vertex
/**
\ref EVT_TRANSFORMED
*/
class VertexTransformed
{
public:
	math::vector3f position; //!< The vertex position
	float RHW; //!< Reciproc W
	video::Color color; //! The vertex color
	math::vector2f texture; //! The vertex texturecoordinate

	//! Default constructor
	VertexTransformed()
	{
	}

	//! Construct from values
	VertexTransformed(float Px, float Py, float Pz, float Prhw, Color _Color, float Tx, float Ty)
		: position(Px, Py, Pz), RHW(Prhw), color(_Color), texture(Tx, Ty)
	{
	}

	//! Construct from multiple values
	VertexTransformed(const math::vector3f& _Pos, float Prhw, Color _Color, const math::vector2f& _TCoords)
		: position(_Pos), RHW(Prhw), color(_Color), texture(_TCoords)
	{
	}

	//! Construct without color
	/**
	Set color to white
	*/
	VertexTransformed(const math::vector3f& _Pos, float Prhw, const math::vector2f& _TCoords)
		: position(_Pos), RHW(Prhw), color(0xFFFFFFFF), texture(_TCoords)
	{
	}

	//! Equality
	bool operator==(const VertexTransformed& other) const
	{
		return ((position == other.position) &&
			(RHW == other.RHW) &&
			(color == other.color) &&
			(texture == other.texture));
	}

	//! Unequality
	bool operator!=(const VertexTransformed& other) const
	{
		return ((position != other.position) ||
			(RHW != other.RHW) ||
			(color != other.color) ||
			(texture != other.texture));
	}
};

//! Vertex with two texture coordinates
/**
\ref EVT_2TCOORDS
*/
class Vertex2TCoords : public Vertex3D
{
public:
	math::vector2f texture2; //!<< The second vertex texture coordinates

	//! Default constructor
	Vertex2TCoords() : Vertex3D()
	{
	}

	//! Construct from single values
	Vertex2TCoords(float Px, float Py, float Pz, Color _Color, float Nx, float Ny, float Nz, float Tx, float Ty, float T2x, float T2y) :
		Vertex3D(Px, Py, Pz, _Color, Nx, Ny, Nz, Tx, Ty), texture2(T2x, T2y)
	{
	}

	//! Construct from combines values
	Vertex2TCoords(const math::vector3f& _Pos, Color _Color, const math::vector3f& _Normal,
		const math::vector2f& _TCoords, const math::vector2f& _TCoords2) :
		Vertex3D(_Pos, _Color, _Normal, _TCoords), texture2(_TCoords2)
	{
	}
	//! Construct without normal
	Vertex2TCoords(const math::vector3f& _Pos, Color _Color,
		const math::vector2f& _TCoords, const math::vector2f& _TCoords2) :
		Vertex3D(_Pos, _Color, math::vector3f(), _TCoords), texture2(_TCoords2)
	{
	}

	//! Construct with identical texture coordinates
	Vertex2TCoords(const math::vector3f& _Pos, Color _Color,
		const math::vector3f& _Normal, const math::vector2f& _TCoords) :
		Vertex3D(_Pos, _Color, _Normal, _TCoords), texture2(_TCoords)
	{
	}

	//! Construct from Vertex3d
	Vertex2TCoords(const Vertex3D& a) : Vertex3D(a)
	{
	}

	//! Equality
	bool operator==(const Vertex2TCoords& other) const
	{
		return ((static_cast<Vertex3D>(*this) == other) &&
			(texture2 == other.texture2));
	}

	//! Unequality
	bool operator!=(const Vertex2TCoords& other) const
	{
		return ((static_cast<Vertex3D>(*this) != other) ||
			(texture2 != other.texture2));
	}
};

//! Vertex with binormal and tangent vector
/**
\ref EVT_TANGENTS
*/
class VertexTangents : public Vertex3D
{
public:
	math::vector3f binormal; //!< The binormal vector
	math::vector3f tangent; //!< The tangent vector

	//! Default constructor
	VertexTangents() : Vertex3D()
	{
	}

	//! Construct from single values
	VertexTangents(float Px, float Py, float Pz, Color _Color, float Nx, float Ny, float Nz, float Tx, float Ty,
		float Tax, float Tay, float Taz, float Bx, float By, float Bz) :
		Vertex3D(Px, Py, Pz, _Color, Nx, Ny, Nz, Tx, Ty), tangent(Tax, Tay, Taz), binormal(Bx, By, Bz)
	{
	}

	//! Construct from combined values
	VertexTangents(const math::vector3f& _Pos, const math::vector3f& _Normal, Color _Color,
		const math::vector2f& _TCoords, const math::vector3f& _Tangent, const math::vector3f& _Binormal) :
		Vertex3D(_Pos, _Color, _Normal, _TCoords), binormal(_Binormal), tangent(_Tangent)
	{
	}

	//! Construct without tangents
	VertexTangents(const math::vector3f& _Pos, const math::vector3f& _Normal, Color _Color,
		const math::vector2f& _TCoords) :
		Vertex3D(_Pos, _Color, _Normal, _TCoords), binormal(math::vector3f()), tangent(math::vector3f())
	{
	}

	//! Construct from Vector3D
	VertexTangents(const Vertex3D& a) : Vertex3D(a)
	{
	}

	//! Equality
	bool operator==(const VertexTangents& other) const
	{
		return ((static_cast<Vertex3D>(*this) == other) &&
			(tangent == other.tangent)) &&
			(binormal == other.binormal);
	}

	//! Unequality
	bool operator!=(const VertexTangents& other) const
	{
		return ((static_cast<Vertex3D>(*this) != other) ||
			(tangent != other.tangent)) ||
			(binormal != other.binormal);
	}
};

//! Vertex with 3D texturecoordinate
/**
\ref EVT_3DTEXCOORD
*/
class Vertex3DTCoord
{
public:
	math::vector3f position; //!< The vertex position
	math::vector3f texture; //!< The 3d texture coordinate

	//! Default constructor
	Vertex3DTCoord()
	{
	}

	//! Construct from single values
	Vertex3DTCoord(float Px, float Py, float Pz, float Tx, float Ty, float Tz) :
		position(Px, Py, Pz), texture(Tx, Ty, Tz)
	{
	}

	//! Construct from combined values
	Vertex3DTCoord(const math::vector3f& _Pos, const math::vector3f& _TCoords)
		: position(_Pos), texture(_TCoords)
	{
	}

	//! Equality
	bool operator==(const Vertex3DTCoord& other) const
	{
		return ((position == other.position) && (texture == other.texture));
	}

	//! Unequality
	bool operator!=(const Vertex3DTCoord& other) const
	{
		return ((position != other.position) || (texture != other.texture));
	}
};

}    

}    


#endif