#ifndef INCLUDED_FOG_DATA_H
#define INCLUDED_FOG_DATA_H
#include "video/ColorFloat.h"

namespace lux
{
namespace video
{

class FogData
{
public:
	enum class EType
	{
		Linear,
		Exp,
		ExpSq,
	};

public:
	bool isActive;

	EType type;

	float start;
	float end;

	float density;

	video::Colorf color;

	FogData() :
		isActive(true),
		type(EType::Linear),
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