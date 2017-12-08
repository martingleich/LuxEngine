#include "scene/components/Fog.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::Fog, lux::scene::SceneComponentType::Fog);

namespace lux
{
namespace scene
{

Fog::Fog()
{
	m_Data.color = video::Colorf(1.0f, 1.0f, 1.0f, 1.0f);
	m_Data.type = video::EFogType::Linear;
	m_Data.start = 0;
	m_Data.end = 0;
	m_Data.density = 1;
}

Fog::~Fog()
{
}

void Fog::SetFogType(video::EFogType type)
{
	m_Data.type = type;
}
video::EFogType Fog::GetFogType() const
{
	return m_Data.type;
}

void Fog::SetDensity(float density)
{
	m_Data.density = density;
}
float Fog::GetDensity() const
{
	return m_Data.density;
}

void Fog::SetStart(float start)
{
	m_Data.start = start;
}
float Fog::GetStart() const
{
	return m_Data.start;
}

void Fog::SetEnd(float end)
{
	m_Data.end = end;
}
float Fog::GetEnd() const
{
	return m_Data.end;
}

void Fog::SetColor(const video::Colorf& color)
{
	m_Data.color = color;
}

const video::Colorf& Fog::GetColor() const
{
	return m_Data.color;
}

video::FogData Fog::GetFogData() const
{
	return m_Data;
}

} // namespace scene
} // namespace lux