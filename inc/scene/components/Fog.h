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
};

class ClassicalFogDescription : public FogDescription
{
public:
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

class LinearFog : public Fog
{
	LX_REFERABLE_MEMBERS_API(LinearFog, LUX_API);
	class LinearFogDescription : public  ClassicalFogDescription
	{
	public:
		video::EFogType GetType() { return video::EFogType::Linear; }
		float GetStart()  { return start; }
		float GetEnd() { return end; }
		float GetDensity() { return 0.0f; }
		video::ColorF GetColor() { return color; }

		float start;
		float end;
		video::ColorF color;
	};

public:
	LUX_API LinearFog();
	LUX_API ~LinearFog();

	LUX_API void SetStart(float start);
	LUX_API float GetStart() const;

	LUX_API void SetEnd(float end);
	LUX_API float GetEnd() const;

	LUX_API void SetColor(const video::ColorF& color);
	LUX_API const video::ColorF& GetColor() const;

	LUX_API FogDescription* GetFogDescription();

private:
	LinearFogDescription m_Data;
};

class ExponentialFog : public Fog
{
	LX_REFERABLE_MEMBERS_API(ExponentialFog, LUX_API);
	class ExponentialFogDescription : public  ClassicalFogDescription
	{
	public:
		video::EFogType GetType() { return video::EFogType::Exp; }
		float GetStart()  { return 0; }
		float GetEnd() { return 0; }
		float GetDensity() { return density; }
		video::ColorF GetColor() { return color; }

		float density;
		video::ColorF color;
	};

public:
	LUX_API ExponentialFog();
	LUX_API ~ExponentialFog();

	LUX_API void SetColor(const video::ColorF& color);
	LUX_API const video::ColorF& GetColor() const;

	LUX_API void SetDensity(float f);
	LUX_API float GetDensity() const;

	LUX_API FogDescription* GetFogDescription();

private:
	ExponentialFogDescription m_Data;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_LUX_SCENE_FOG_H