#ifndef INCLUDED_FOG_DATA_H
#define INCLUDED_FOG_DATA_H
#include "video/Color.h"

namespace lux
{
namespace video
{

enum class EFogType
{
	Linear,
	Exp,
	ExpSq,
};

class FogData
{
public:
	EFogType type;

	float start;
	float end;

	float density;

	video::Colorf color;

	FogData() :
		type(EFogType::Linear),
		start(0.0f),
		end(100.0f),
		density(0.0f),
		color(0.1f, 0.1f, 0.1f)
	{
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_FOG_DATA_H