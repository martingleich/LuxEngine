#include "scene/components/Fog.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::GlobalFog, "lux.comp.GlobalFog");

namespace lux
{
namespace scene
{

GlobalFog::GlobalFog()
{
	m_Data.color = video::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
	m_Data.type = video::EFogType::Linear;
	m_Data.start = 0;
	m_Data.end = 0;
	m_Data.density = 1;
}

GlobalFog::~GlobalFog()
{
}

void GlobalFog::SetFogType(video::EFogType type)
{
	m_Data.type = type;
}
video::EFogType GlobalFog::GetFogType() const
{
	return m_Data.type;
}

void GlobalFog::SetDensity(float density)
{
	m_Data.density = density;
}
float GlobalFog::GetDensity() const
{
	return m_Data.density;
}

void GlobalFog::SetStart(float start)
{
	m_Data.start = start;
}
float GlobalFog::GetStart() const
{
	return m_Data.start;
}

void GlobalFog::SetEnd(float end)
{
	m_Data.end = end;
}
float GlobalFog::GetEnd() const
{
	return m_Data.end;
}

void GlobalFog::SetColor(const video::ColorF& color)
{
	m_Data.color = color;
}

const video::ColorF& GlobalFog::GetColor() const
{
	return m_Data.color;
}

video::FogData GlobalFog::GetFogData() const
{
	return m_Data;
}

} // namespace scene
} // namespace lux