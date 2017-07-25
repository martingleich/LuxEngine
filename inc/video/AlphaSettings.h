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

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_ALPHA_SETTINGS_H