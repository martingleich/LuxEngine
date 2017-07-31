#ifndef INCLUDED_VIDEO_ENUMS_H
#define INCLUDED_VIDEO_ENUMS_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{

enum class ELighting : u8
{
	Disabled = 0,
	Ambient = 1,
	Emissive = 2,
	Diffuse = 4,
	Specular = 8,
	AmbientEmit = Ambient | Emissive,
	DiffSpec = Diffuse | Specular,

	Enabled = Ambient | Emissive | Diffuse | Specular,
};


// Blendfaktor f�r Alphablending
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
// dem Blendfaktor verkn�pft.
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

} // namespace video
DECLARE_FLAG_CLASS(video::ELighting);
} // namespace lux

#endif // #ifndef INCLUDED_ALPHA_SETTINGS_H
