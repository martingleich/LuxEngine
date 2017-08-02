#ifndef INCLUDED_S3DVERTEX_H
#define INCLUDED_S3DVERTEX_H
#include "math/Vector2.h"
#include "math/vector3.h"
#include "video/Color.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// REMARK
// The types in this file are directly used to access memory, meaning
// their data layout _must_ never change.
// Furthermore they must be trivial types.
// Meaning they must not have a destructur, or any virtual methods.
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
namespace lux
{
namespace video
{

//! The standard 3d vertex
/**
\ref VertexFormat::STANDARD
*/
struct Vertex3D
{
	math::Vector3F position; //!< The vertex position
	math::Vector3F normal; //!< The vertex normal
	video::Color color; //!< The vertex color
	math::Vector2F texture; //!< The vertex texturecoords

	//! Default constructor
	Vertex3D()
	{
	}

	//! Construct from single values
	Vertex3D(float Px, float Py, float Pz, Color _Color, float Nx, float Ny, float Nz, float Tx, float Ty) :
		position(Px, Py, Pz), normal(Nx, Ny, Nz), color(_Color), texture(Tx, Ty)
	{
	}

	//! Construct from combined values
	Vertex3D(const math::Vector3F& _Pos, Color _Color, const math::Vector3F& _Normal, const math::Vector2F& _TCoords)
		: position(_Pos), normal(_Normal), color(_Color), texture(_TCoords)
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

//! The vertex with position only
/**
\ref VertexFormat::POS_ONLY
*/
struct VertexPosOnly
{
	math::Vector3F position; //!< The vertex position

	//! Default constructor
	VertexPosOnly()
	{
	}

	//! Construct from single values
	VertexPosOnly(float Px, float Py, float Pz) :
		position(Px, Py, Pz)
	{
	}

	//! Construct from combined values
	VertexPosOnly(const math::Vector3F& _Pos)
		: position(_Pos)
	{
	}

	//! Equality
	bool operator==(const VertexPosOnly& other) const
	{
		return ((position == other.position));
	}

	//! Unequality
	bool operator!=(const VertexPosOnly& other) const
	{
		return ((position != other.position));
	}

	//! Smaller
	bool operator<(const VertexPosOnly& other) const
	{
		return ((position < other.position));
	}
};

//! The vertex with transformed position only 
/**
\ref VertexFormat::POSW_ONLY
*/
struct VertexPosWOnly
{
	math::Vector3F position; //!< The vertex position
	float rhw;

	//! Default constructor
	VertexPosWOnly()
	{
	}

	//! Construct from single values
	VertexPosWOnly(float Px, float Py, float Pz, float _rhw) :
		position(Px, Py, Pz), rhw(_rhw)
	{
	}

	//! Construct from combined values
	VertexPosWOnly(const math::Vector3F& _Pos, float _rhw) :
		position(_Pos), rhw(_rhw)
	{
	}

	//! Equality
	bool operator==(const VertexPosWOnly& other) const
	{
		return ((position == other.position) && rhw == other.rhw);
	}

	//! Unequality
	bool operator!=(const VertexPosWOnly& other) const
	{
		return ((position != other.position) && rhw != other.rhw);
	}

	//! Smaller
	bool operator<(const VertexPosWOnly& other) const
	{
		return ((position < other.position) ||
			(position == other.position) && (rhw < other.rhw));
	}
};

//! A simple 2d vertex
/**
\ref VertexFormat::STANDARD_2D
*/
struct Vertex2D
{
	math::Vector2F position; //!< The vertex position
	video::Color color; //!< The vertex color
	math::Vector2F texture; //! The vertex texturecoords

	//! Default constructor
	Vertex2D()
	{
	}

	//! Construct from single values
	Vertex2D(float Px, float Py, Color _Color, float Tx, float Ty) :
		position(Px, Py), color(_Color), texture(Tx, Ty)
	{
	}

	//! Construct from combined values
	Vertex2D(float Px, float Py, Color _Color) :
		position(Px, Py), color(_Color), texture(0.0f, 0.0f)
	{
	}

	//! Full constructor
	Vertex2D(const math::Vector2F& _Pos, Color _Color, const math::Vector2F& _TCoords)
		: position(_Pos), color(_Color), texture(_TCoords)
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
\ref VertexFormat::TRANSFORMED
*/
struct VertexTransformed
{
	math::Vector3F position; //!< The vertex position
	float RHW; //!< Reciproc W
	video::Color color; //! The vertex color
	math::Vector2F texture; //! The vertex texturecoordinate

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
	VertexTransformed(const math::Vector3F& _Pos, float Prhw, Color _Color, const math::Vector2F& _TCoords)
		: position(_Pos), RHW(Prhw), color(_Color), texture(_TCoords)
	{
	}

	//! Construct without color
	/**
	Set color to white
	*/
	VertexTransformed(const math::Vector3F& _Pos, float Prhw, const math::Vector2F& _TCoords)
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
\ref VertexFormat::TWO_TEXTURE
*/
struct Vertex2TCoords
{
	math::Vector3F position; //!< The vertex position
	math::Vector3F normal; //!< The vertex normal
	video::Color color; //!< The vertex color
	math::Vector2F texture; //!< The vertex texturecoords
	math::Vector2F texture2; //!<< The second vertex texture coordinates

	//! Default constructor
	Vertex2TCoords()
	{
	}

	//! Construct from combines values
	Vertex2TCoords(
			const math::Vector3F& _Pos,
			Color _Color,
			const math::Vector3F& _Normal,
			const math::Vector2F& _TCoords,
			const math::Vector2F& _TCoords2) :
		position(_Pos),
		normal(_Normal),
		color(_Color),
		texture(_TCoords),
		texture2(_TCoords2)
	{
	}

	//! Construct from single values
	Vertex2TCoords(
			float Px, float Py, float Pz, 
			Color _Color,
			float Nx, float Ny, float Nz,
			float Tx, float Ty,
			float T2x, float T2y) :
		position(Px, Py, Pz),
		normal(Nx, Ny, Nz),
		color(_Color),
		texture(Tx, Ty),
		texture2(T2x, T2y)
	{
	}

	//! Construct without normal
	Vertex2TCoords(const math::Vector3F& _Pos, Color _Color,
		const math::Vector2F& _TCoords, const math::Vector2F& _TCoords2) :
		position(_Pos),
		color(_Color),
		texture(_TCoords),
		texture2(_TCoords2)
	{
	}

	//! Construct with identical texture coordinates
	Vertex2TCoords(const math::Vector3F& _Pos, Color _Color,
		const math::Vector3F& _Normal, const math::Vector2F& _TCoords) :
		position(_Pos),
		normal(_Normal),
		color(_Color),
		texture(_TCoords),
		texture2(_TCoords)
	{
	}

	//! Equality
	bool operator==(const Vertex2TCoords& other) const
	{
		return 
			position == other.position &&
			normal == other.normal &&
			color == other.color &&
			texture == other.texture &&
			texture2 == other.texture2;
	}

	//! Unequality
	bool operator!=(const Vertex2TCoords& other) const
	{
		return !(*this == other);
	}
};

//! Vertex with binormal and tangent vector
/**
\ref VertexFormat::TANGENTS
*/
struct VertexTangents
{
	math::Vector3F position; //!< The vertex position
	math::Vector3F normal; //!< The vertex normal
	video::Color color; //!< The vertex color
	math::Vector2F texture; //!< The vertex texturecoords
	math::Vector3F tangent; //!< The tangent vector
	math::Vector3F binormal; //!< The binormal vector

	//! Default constructor
	VertexTangents()
	{
	}

	//! Construct from single values
	VertexTangents(
			float Px, float Py, float Pz,
			Color _Color,
			float Nx, float Ny, float Nz,
			float Tx, float Ty,
			float Tax, float Tay, float Taz,
			float Bx, float By, float Bz) :
		position(Px, Py, Pz),
		normal(Nx, Ny, Nz),
		color(_Color),
		texture(Tx, Ty),
		tangent(Tax, Tay, Taz),
		binormal(Bx, By, Bz)
	{
	}

	//! Construct from combined values
	VertexTangents(const math::Vector3F& _Pos, const math::Vector3F& _Normal, Color _Color,
		const math::Vector2F& _TCoords, const math::Vector3F& _Tangent, const math::Vector3F& _Binormal) :
		position(_Pos), normal(_Normal), color(_Color), texture(_TCoords), tangent(_Tangent), binormal(_Binormal)
	{
	}

	//! Construct without tangents
	VertexTangents(const math::Vector3F& _Pos, const math::Vector3F& _Normal, Color _Color,
		const math::Vector2F& _TCoords) :
		position(_Pos), normal(_Normal), color(_Color), texture(_TCoords)
	{
	}

	//! Equality
	bool operator==(const VertexTangents& other) const
	{
		return
			position == other.position &&
			normal == other.normal &&
			color == other.color &&
			texture == other.texture &&
			tangent == other.tangent &&
			binormal == other.binormal;
	}

	//! Unequality
	bool operator!=(const VertexTangents& other) const
	{
		return !(*this == other);
	}
};

//! Vertex with 3D texturecoordinate
/**
\ref VertexFormat::TEXTURE_3D
*/
struct Vertex3DTCoord
{
	math::Vector3F position; //!< The vertex position
	math::Vector3F texture; //!< The 3d texture coordinate

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
	Vertex3DTCoord(const math::Vector3F& _Pos, const math::Vector3F& _TCoords)
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
