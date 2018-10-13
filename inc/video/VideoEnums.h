#ifndef INCLUDED_LUX_VIDEO_ENUMS_H
#define INCLUDED_LUX_VIDEO_ENUMS_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{

enum class ELightingFlag : u8
{
	Disabled = 0,
	AmbientEmit = 1,
	DiffSpec = 2,

	Enabled = AmbientEmit | DiffSpec,
};

// Blendfaktor für Alphablending
enum class EBlendFactor : u8
{
	// Alpha ist immer 0
	Zero,
	// Alpha ist immer 1
	One,
	// Alpha ist gleich Quellalpha
	SrcAlpha,
	// Alpha ist gleich 1-Quellalpha
	OneMinusSrcAlpha,
	// Alpha ist gleich Zielalpha
	DstAlpha,
	// Alpha ist gleich 1-Zielalpha
	OneMinusDstAlpha,
};

// Wie werden die alte und neue Farbe nach Multiplikation mit
// dem Blendfaktor verknüpft.
// Alle Operationen erfolgen Komponentenweise
enum class EBlendOperator : u8
{
	// Es findet kein Alphablending stat
	None,
	// Alte Farbe + Neue Farbe = Ergebnis
	Add,
	// Neue Farbe - Alte Farbe = Ergebnis
	Subtract,
	// Alte Farbe - Neue Farbe = Ergebnis
	RevSubtract,
	// Min(Alte Farbe, Neue Farbe) = Ergebnis
	Min,
	// Max(Alte Farbe, Neue Farbe) = Ergebnis
	Max,
};

enum class EStencilOperator : u8
{
	Keep,
	Zero,
	Replace,
	Invert,
	Increment,
	Decrement,
	IncrementSat,
	DecrementSat,
};

enum class EFaceWinding
{
	CCW,
	CW,
	ANY,
};

inline EFaceWinding FlipWinding(EFaceWinding winding)
{
	switch(winding){
	case EFaceWinding::CCW: return EFaceWinding::CW;
	case EFaceWinding::CW: return EFaceWinding::CCW;
	case EFaceWinding::ANY: return EFaceWinding::ANY;
	}
	return EFaceWinding::ANY;
}

enum class EFaceSide
{
	Front,
	Back,
	None,
	FrontAndBack,
};

inline EFaceSide FlipFaceSide(EFaceSide side)
{
	if(side == EFaceSide::Front)
		return EFaceSide::Back;
	if(side == EFaceSide::Back)
		return EFaceSide::Front;
	return side;
}

inline EFaceSide FlipCulling(EFaceSide cull)
{
	if(cull == EFaceSide::Front)
		return EFaceSide::Back;
	if(cull == EFaceSide::Back)
		return EFaceSide::Front;
	if(cull == EFaceSide::None)
		return EFaceSide::FrontAndBack;
	if(cull == EFaceSide::FrontAndBack)
		return EFaceSide::None;
	return cull;
}

//! Different Primitive Types
enum class EPrimitiveType
{
	//! One point per vertex
	Points,

	//! One line through all vertices
	LineStrip,

	//! Two following vertices, make a line
	Lines,

	//! After the first two vertices, every one creates a new triangle,
	//! the two older ones and the the create a triangle
	TriangleStrip,

	//! A triangle fan, first point in center
	TriangleFan,

	//! Three vertices create a triangle
	Triangles,
};

inline int GetPrimitiveCount(EPrimitiveType type, int pointCount)
{
	switch(type) {
	case video::EPrimitiveType::Lines:
		return pointCount / 2;
	case video::EPrimitiveType::LineStrip:
		return pointCount > 0 ? pointCount - 1 : 0;
	case video::EPrimitiveType::Points:
		return pointCount;
	case video::EPrimitiveType::Triangles:
		return pointCount / 3;
	case video::EPrimitiveType::TriangleFan:
	case video::EPrimitiveType::TriangleStrip:
		return pointCount > 2 ? pointCount - 2 : 0;
	};

	return 0;
}

inline int GetPointCount(EPrimitiveType type, int primitiveCount)
{
	switch(type) {
	case video::EPrimitiveType::Lines:
		return primitiveCount * 2;
	case video::EPrimitiveType::LineStrip:
		return primitiveCount < 2 ? primitiveCount * 2 : primitiveCount + 1;
	case video::EPrimitiveType::Points:
		return primitiveCount;
	case video::EPrimitiveType::Triangles:
		return primitiveCount * 3;
	case video::EPrimitiveType::TriangleFan:
	case video::EPrimitiveType::TriangleStrip:
		return primitiveCount < 2 ? primitiveCount * 3 : primitiveCount + 2;
	};

	return 0;
}

enum class EShaderLanguage
{
	HLSL,
	Unknown
};

enum class EShaderType
{
	Vertex,
	Pixel,
	Unknown
};

enum class EComparisonFunc : u8
{
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,
};

enum class EColorPlane : u8 // enum class flag at end of file
{
	None = 0,
	Alpha = 1,
	Red = 2,
	Green = 4,
	Blue = 8,
	RGB = Red | Green | Blue,
	All = RGB | Alpha,
};

enum class EDrawMode : u8
{
	Fill,
	Wire,
	Point,
};

enum class ETextureOperator : u8
{
	Disable,
	SelectArg1,
	SelectArg2,
	Modulate,
	Add,
	AddSigned,
	AddSmoth,
	Subtract,
	Blend,
	Dot,
};

enum class ETextureArgument : u8
{
	Current,
	Texture,
	Diffuse,
	AlphaRep,
};

//! How are texturecoordinates outside of [0,1] handeled
enum class ETextureRepeat : u8
{
	Wrap = 0,
	Mirror,
	Clamp,
	MirrorOnce,
	Border,
};

//! The type of the hardware buffer
enum class EHardwareBufferType
{
	Index = 0, //!< A index buffer
	Vertex, //!< A vertex buffer
};

//! How should data be saved in hardware
enum class EHardwareBufferMapping
{
	Static,     //!< data isnt changed often, saved in static area
	Dynamic,    //!< data is high frequent, saved in dynamic area
};

//! The data saved in a index buffer
enum class EIndexFormat
{
	Bit16, //!< 16 Bit per index
	Bit32, //!< 32 Bit per index
};

struct ZStencilFormat
{
	ZStencilFormat(u8 z, u8 s, u8 str) :
		zBits(z),
		sBits(s),
		stride(str)
	{
	}
	ZStencilFormat() :
		zBits(0), sBits(0), stride(0)
	{
	}
	u8 zBits;
	u8 sBits;
	u8 stride;

	bool operator==(const ZStencilFormat& other) const
	{
		return zBits == other.zBits && sBits == other.sBits && stride == other.stride;
	}

	bool operator!=(const ZStencilFormat& other) const
	{
		return !(*this == other);
	}
};

struct StencilMode
{
	u32 ref = 0;

	u32 readMask = 0xFFFFFFFF;
	u32 writeMask = 0xFFFFFFFF;

	EComparisonFunc test = EComparisonFunc::Always;

	EStencilOperator pass = EStencilOperator::Keep;
	EStencilOperator fail = EStencilOperator::Keep;
	EStencilOperator zFail = EStencilOperator::Keep;

	EStencilOperator passCCW = EStencilOperator::Keep;
	EStencilOperator failCCW = EStencilOperator::Keep;
	EStencilOperator zFailCCW = EStencilOperator::Keep;

	bool IsTwoSided() const
	{
		return
			pass != passCCW ||
			fail != failCCW ||
			zFail != zFailCCW;
	}

	bool IsEnabled() const
	{
		return !(test == EComparisonFunc::Always &&
			pass == EStencilOperator::Keep &&
			fail == EStencilOperator::Keep &&
			zFail == EStencilOperator::Keep &&
			passCCW == EStencilOperator::Keep &&
			failCCW == EStencilOperator::Keep &&
			zFailCCW == EStencilOperator::Keep);
	}
};

struct AlphaBlendMode
{
	AlphaBlendMode() :
		srcFactor(EBlendFactor::One),
		dstFactor(EBlendFactor::Zero),
		blendOperator(EBlendOperator::None)
	{
	}

	AlphaBlendMode(EBlendFactor src, EBlendFactor dst, EBlendOperator op) :
		srcFactor(src),
		dstFactor(dst),
		blendOperator(op)
	{
	}

	EBlendFactor srcFactor;
	EBlendFactor dstFactor;
	EBlendOperator blendOperator;

	bool IsTransparent() const
	{
		return dstFactor != EBlendFactor::Zero && blendOperator != EBlendOperator::None;
	}

	bool operator==(const AlphaBlendMode& other) const
	{
		return srcFactor == other.srcFactor && dstFactor == other.dstFactor && blendOperator == other.blendOperator;
	}

	bool operator!=(const AlphaBlendMode& other) const
	{
		return !(*this == other);
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_ALPHA_SETTINGS_H
