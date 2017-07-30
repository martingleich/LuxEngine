#ifndef INCLUDED_LUX_DEVICE_NULL_H
#define INCLUDED_LUX_DEVICE_NULL_H
#include "LuxEngine/LuxDevice.h"

namespace lux
{

class LuxDeviceNull : public LuxDevice
{
void LuxDeviceWin32::BuildImageSystem()
{
	if(video::ImageSystem::Instance()) {
		log::Warning("Image system already built.");
		return;
	}

	log::Info("Build Image System.");
	video::ImageSystem::Initialize();
}

void LuxDeviceWin32::BuildGUIEnvironment()
{
	if(m_GUIEnv != nullptr) {
		log::Warning("Gui environment already built.");
		return;
	}

	log::Info("Build GUI Environment.");
	m_GUIEnv = LUX_NEW(gui::GUIEnvironmentImpl);
}

void LuxDeviceWin32::BuildAll(const video::DriverConfig& config)
{
	BuildWindow(config.width, config.height, "Window");
	BuildInputSystem();
	BuildVideoDriver(config);
	BuildSceneManager();
	BuildGUIEnvironment();
}

};

} // namespace lux

#endif // #ifndef INCLUDED_LUX_DEVICE_NULL_H