#include "video/VideoDriverNull.h"
#include "video/SceneValuesImpl.h"
#include "video/RenderStatistics.h"

namespace lux
{
namespace video
{

VideoDriverNull::VideoDriverNull(core::ReferableFactory* refFactory) :
	m_AmbientColor(video::Color::Black)
{
	m_RefFactory = refFactory;

	m_RenderStatistics = LUX_NEW(RenderStatistics);

	m_SceneValues = LUX_NEW(scene::SceneValuesImpl);
	m_SceneValueAmbient = m_SceneValues->AddParam("ambient", core::Type::ColorF);
}

VideoDriverNull::~VideoDriverNull()
{
}

void VideoDriverNull::Init(const DriverConfig& config, gui::Window* window)
{
	LUX_UNUSED(window);
	m_Config = config;
}

void VideoDriverNull::SetAmbient(Colorf ambient)
{
	m_AmbientColor = ambient;
	m_SceneValues->SetParamValue(m_SceneValueAmbient, &m_AmbientColor);
}

Colorf VideoDriverNull::GetAmbient() const
{
	return m_AmbientColor;
}

void VideoDriverNull::AddLight(const LightData& light)
{
	if(m_LightList.Size() == GetDeviceCapability(EDriverCaps::MaxLights))
		throw core::Exception("Too many lights");

	m_LightList.PushBack(light);

	m_SceneValues->SetLight(m_LightList.Size()-1, light);
}

size_t VideoDriverNull::GetLightCount() const
{
	return m_LightList.Size();
}

void VideoDriverNull::ClearLights()
{
	m_LightList.Clear();
	m_SceneValues->ClearLights();
}

StrongRef<RenderStatistics> VideoDriverNull::GetRenderStatistics() const
{
	return m_RenderStatistics;
}

StrongRef<scene::SceneValues> VideoDriverNull::GetSceneValues() const
{
	return m_SceneValues;
}

EVideoDriver VideoDriverNull::GetVideoDriverType() const
{
	return EVideoDriver::Null;
}

const DriverConfig& VideoDriverNull::GetConfig() const
{
	return m_Config;
}

u32 VideoDriverNull::GetDeviceCapability(EDriverCaps Capability) const
{
	return m_DriverCaps[(u32)Capability];
}

}
}