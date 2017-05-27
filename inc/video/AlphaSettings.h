#ifndef INCLUDED_ALPHA_SETTINGS_H
#define INCLUDED_ALPHA_SETTINGS_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{

// Blendfaktor für Alphablending
enum class EBlendFactor
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

//! Uses a texture blending factor alpha values.
inline bool HasTextureBlendAlpha(const EBlendFactor factor)
{
	switch(factor) {
	case EBlendFactor::SrcAlpha:
	case EBlendFactor::OneMinusSrcAlpha:
	case EBlendFactor::DstAlpha:
	case EBlendFactor::OneMinusDstAlpha:
		return true;
	default:
		return false;
	}
}

// Wie werden die alte und neue Farbe nach Multiplikation mit
// dem Blendfaktor verknüpft.
// Alle Operationen erfolgen Komponentenweise
enum class EBlendOperator
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

//! The source of alpha values
enum class EAlphaSource
{
	VertexColor, //!< From each vertex color
	Texture, //!< From the current texture
	VertexAndTexture //! From the vertex color and the texture combined
};

class AlphaBlendSettings
{
public:
	EBlendFactor SrcBlend;
	EBlendFactor DstBlend;
	EBlendOperator Operator;
	EAlphaSource AlphaSrc;

	AlphaBlendSettings(EBlendFactor src, EBlendFactor dst, EBlendOperator op, EAlphaSource alpha) :
		SrcBlend(src),
		DstBlend(dst),
		Operator(op),
		AlphaSrc(alpha)
	{
	}
	AlphaBlendSettings() :
		SrcBlend(EBlendFactor::One),
		DstBlend(EBlendFactor::Zero),
		Operator(EBlendOperator::None),
		AlphaSrc(EAlphaSource::Texture)
	{
	}

	static AlphaBlendSettings Disabled()
	{
		return AlphaBlendSettings(EBlendFactor::One, EBlendFactor::Zero, EBlendOperator::None, EAlphaSource::Texture);
	}

	u32 Pack() const
	{
		// 0000000000000000aaaaoooossssdddd
		return 
			(((u32)AlphaSrc << 12) & 0xF000) |
			(((u32)Operator <<  8) & 0x0F00) |
			(((u32)SrcBlend <<  4) & 0x00F0) |
			(((u32)DstBlend <<  0) & 0x000F);
	}

	void Unpack(u32 packed)
	{
		AlphaSrc = EAlphaSource((packed & 0x0000F000) >> 12);
		Operator = EBlendOperator((packed & 0x00000F00) >> 8);
		SrcBlend = EBlendFactor((packed & 0x000000F0) >> 4);
		DstBlend = EBlendFactor((packed & 0x0000000F));
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_ALPHA_SETTINGS_H