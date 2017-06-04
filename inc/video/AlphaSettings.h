#ifndef INCLUDED_ALPHA_SETTINGS_H
#define INCLUDED_ALPHA_SETTINGS_H
#include "core/LuxBase.h"

namespace lux
{
namespace video
{

// Blendfaktor f�r Alphablending
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
// dem Blendfaktor verkn�pft.
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

class AlphaBlendSettings
{
public:
	EBlendFactor SrcBlend;
	EBlendFactor DstBlend;
	EBlendOperator Operator;

	AlphaBlendSettings(EBlendFactor src, EBlendFactor dst, EBlendOperator op) :
		SrcBlend(src),
		DstBlend(dst),
		Operator(op)
	{
	}
	AlphaBlendSettings() :
		SrcBlend(EBlendFactor::One),
		DstBlend(EBlendFactor::Zero),
		Operator(EBlendOperator::None)
	{
	}

	static AlphaBlendSettings Disabled()
	{
		return AlphaBlendSettings(EBlendFactor::One, EBlendFactor::Zero, EBlendOperator::None);
	}

	u32 Pack() const
	{
		// 00000000000000000000oooossssdddd
		return
			(((u32)Operator << 8) & 0x0F00) |
			(((u32)SrcBlend << 4) & 0x00F0) |
			(((u32)DstBlend << 0) & 0x000F);
	}

	void Unpack(u32 packed)
	{
		Operator = EBlendOperator((packed & 0x00000F00) >> 8);
		SrcBlend = EBlendFactor((packed & 0x000000F0) >> 4);
		DstBlend = EBlendFactor((packed & 0x0000000F));
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_ALPHA_SETTINGS_H