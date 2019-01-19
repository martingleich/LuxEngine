#include "scene/components/Fog.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::LinearFog, "lux.comp.LinearFog");
LX_REFERABLE_MEMBERS_SRC(lux::scene::ExponentialFog, "lux.comp.ExpFog");

namespace lux
{
namespace scene
{

LinearFog::LinearFog()
{
	m_Data.color = video::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
	m_Data.start = 0;
	m_Data.end = 0;
}

LinearFog::~LinearFog()
{
}

void LinearFog::SetStart(float start)
{
	m_Data.start = start;
}
float LinearFog::GetStart() const
{
	return m_Data.start;
}

void LinearFog::SetEnd(float end)
{
	m_Data.end = end;
}
float LinearFog::GetEnd() const
{
	return m_Data.end;
}

void LinearFog::SetColor(const video::ColorF& color)
{
	m_Data.color = color;
}

const video::ColorF& LinearFog::GetColor() const
{
	return m_Data.color;
}

FogDescription* LinearFog::GetFogDescription()
{
	return &m_Data;
}

ExponentialFog::ExponentialFog()
{
	m_Data.color = video::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
	m_Data.density = 0;
}

ExponentialFog::~ExponentialFog()
{
}

void ExponentialFog::SetDensity(float density)
{
	m_Data.density = density;
}

float ExponentialFog::GetDensity() const
{
	return m_Data.density;
}

void ExponentialFog::SetColor(const video::ColorF& color)
{
	m_Data.color = color;
}

const video::ColorF& ExponentialFog::GetColor() const
{
	return m_Data.color;
}

FogDescription* ExponentialFog::GetFogDescription()
{
	return &m_Data;
}

} // namespace scene
} // namespace lux