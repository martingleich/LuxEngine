#ifndef INCLUDED_SCENE_FOG_H
#define INCLUDED_SCENE_FOG_H
#include "scene/Component.h"
#include "video/FogData.h"

namespace lux
{
namespace scene
{

class Fog : public Component
{
public:
	LX_REFERABLE_MEMBERS_API(LUX_API);

	LUX_API Fog();
	LUX_API virtual ~Fog();

	LUX_API virtual void SetFogType(video::EFogType type);
	LUX_API virtual video::EFogType GetFogType() const;

	LUX_API virtual void SetDensity(float density);
	LUX_API virtual float GetDensity() const;

	LUX_API virtual void SetStart(float start);
	LUX_API virtual float GetStart() const;

	LUX_API virtual void SetEnd(float end);
	LUX_API virtual float GetEnd() const;

	LUX_API virtual void SetColor(const video::Colorf& color);
	LUX_API virtual const video::Colorf& GetColor() const;

	LUX_API virtual video::FogData GetFogData() const;

private:
	video::FogData m_Data;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_SCENE_FOG_H