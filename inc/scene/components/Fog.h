#ifndef INCLUDED_LUX_SCENE_FOG_H
#define INCLUDED_LUX_SCENE_FOG_H
#include "scene/Component.h"
#include "video/FogData.h"

namespace lux
{
namespace scene
{

class GlobalFog : public Component
{
	LX_REFERABLE_MEMBERS_API(GlobalFog, LUX_API);
public:
	LUX_API GlobalFog();
	LUX_API virtual ~GlobalFog();

	LUX_API virtual void SetFogType(video::EFogType type);
	LUX_API virtual video::EFogType GetFogType() const;

	LUX_API virtual void SetDensity(float density);
	LUX_API virtual float GetDensity() const;

	LUX_API virtual void SetStart(float start);
	LUX_API virtual float GetStart() const;

	LUX_API virtual void SetEnd(float end);
	LUX_API virtual float GetEnd() const;

	LUX_API virtual void SetColor(const video::ColorF& color);
	LUX_API virtual const video::ColorF& GetColor() const;

	LUX_API virtual video::FogData GetFogData() const;

private:
	video::FogData m_Data;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_FOG_H