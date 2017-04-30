#ifndef INCLUDED_VIDEO_DRIVER_NULL_H
#define INCLUDED_VIDEO_DRIVER_NULL_H
#include "video/VideoDriver.h"
#include "core/ReferableFactory.h"
#include "core/Timer.h"
#include "video/RenderStatistics.h"

namespace lux
{
namespace video
{

class VideoDriverNull : public VideoDriver
{
public:
	VideoDriverNull(core::Timer* timer, core::ReferableFactory* refFactory);
	~VideoDriverNull();

	virtual bool Init(const DriverConfig& config, gui::Window* Window);

	virtual void SetAmbient(Colorf ambient);
	virtual Colorf GetAmbient() const;

	virtual bool AddLight(const LightData& light);
	virtual size_t GetLightCount() const;
	virtual void ClearLights();

	virtual StrongRef<RenderStatistics> GetRenderStatistics() const;
	virtual StrongRef<scene::SceneValues> GetSceneValues() const;

	virtual EVideoDriver GetVideoDriverType() const;
	virtual const DriverConfig& GetConfig() const;
	
	virtual u32 GetDeviceCapability(EDriverCaps Capability) const;

protected:
	core::array<LightData> m_LightList;

	Colorf m_AmbientColor;

	u32 m_SceneValueAmbient;

	StrongRef<core::Timer> m_Timer;
	StrongRef<core::ReferableFactory> m_RefFactory;
	StrongRef<RenderStatistics> m_RenderStatistics;
	StrongRef<scene::SceneValues> m_SceneValues;

	DriverConfig m_Config;

	u32 m_DriverCaps[(u32)EDriverCaps::EDriverCaps_Count];
};

}
}

#endif // #ifndef INCLUDED_VIDEO_DRIVER_NULL_H