#ifndef INCLUDED_SMATERIALLAYER_H
#define INCLUDED_SMATERIALLAYER_H

#include "core/lxTypes.h"

namespace lux
{
namespace video
{

class BaseTexture;
class Texture;
class CubeTexture;

// Wie sollen Texturenkoordinaten außerhalb von [0; 1] behandelt werden
enum ETextureRepeat
{
	// Texturkoordinaten, die auf der einen Seite rausgehen kommen auf der anderen wieder rein
	ETR_WRAP = 0,
	// Ähnlich ETR_WRAP nur wird die Textur abwechselnd Orginalegetreu und gespiegelt angezeigt
	ETR_MIRROR,
	// Texturkoordinaten > 1 werden zu 1, Texturkoordinaten < 0 werden zu 0
	ETR_CLAMP,
	// Es wird der Betrag der Texturkoordinaten benutzt, danach wie ETR_CLAMP
	ETR_MIRRORONCE,
};

//Die Klasse für eine Texturebene des Materials
class MaterialLayer
{
public:
	BaseTexture*  texture;
	struct RepeatMode
	{
		ETextureRepeat u : 3;
		ETextureRepeat v : 3;
		RepeatMode& operator=(ETextureRepeat r)
		{
			u = v = r;
			return *this;
		}

		bool operator==(RepeatMode other) const
		{
			return u == other.u && v == other.v;
		}

		bool operator!=(RepeatMode other) const
		{
			return !(*this == other);
		}

		RepeatMode() : u(ETR_WRAP), v(ETR_WRAP)
		{
		}
		RepeatMode(ETextureRepeat r) : u(r), v(r)
		{
		}
		RepeatMode(ETextureRepeat _u, ETextureRepeat _v) : u(_u), v(_v)
		{
		}
	};

	RepeatMode repeat;

	//Konstruktor
	MaterialLayer() :
		texture(nullptr),
		repeat(ETR_WRAP, ETR_WRAP)
	{
	}

	//Kopierkonstuktor
	MaterialLayer(const MaterialLayer& other)
	{
		*this = other;
	}

	//Destuktor
	~MaterialLayer()
	{
	}

	//Zuweisungsoperator
	MaterialLayer& operator=(const MaterialLayer& other)
	{
		if(this == &other)
			return *this;

		texture = other.texture;
		repeat = other.repeat;

		return *this;
	}

	//Ungleichheitsopeartor
	inline bool operator!=(const MaterialLayer& other) const
	{
		return texture != other.texture || repeat != other.repeat;
	}

	//Gleichheitoperator
	inline bool operator==(const MaterialLayer& other) const
	{
		return !(other != *this);
	}
};


} // !namespace video

namespace core
{
template<> inline Type GetTypeInfo<video::MaterialLayer>() { return Type::Texture; }
} // namespace core

} // !namespace lux

#endif // !INCLUDED_SMATERIALLAYER_H
