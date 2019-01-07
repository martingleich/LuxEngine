#ifndef INCLUDED_LUX_SCENE_FOG_H
#define INCLUDED_LUX_SCENE_FOG_H
#include "scene/Component.h"
#include "video/FogData.h"

namespace lux
{
namespace scene
{

class FogDescription
{
public:
	virtual ~FogDescription() {}
	virtual video::EFogType GetType() = 0;
	virtual float GetStart() = 0;
	virtual float GetEnd() = 0;
	virtual float GetDensity() = 0;
	virtual video::ColorF GetColor() = 0;
};

class Fog : public Component
{
public:
	virtual FogDescription* GetFogDescription() = 0;
};

class GlobalFog : public Fog
{
	LX_REFERABLE_MEMBERS_API(GlobalFog, LUX_API);
	class GlobalFogDescription : public FogDescription
	{
	public:
		video::EFogType GetType() { return type; }
		float GetStart() { return start; }
		float GetEnd() { return end; }
		float GetDensity() { return density; }
		video::ColorF GetColor() { return color; }

		video::EFogType type;

		float start;
		float end;

		float density;

		video::ColorF color;
	};

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

	LUX_API FogDescription* GetFogDescription();

private:
	GlobalFogDescription m_Data;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_FOG_H