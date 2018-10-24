#ifndef INCLUDED_LUX_LUX_DEVICE_NULL_H
#define INCLUDED_LUX_LUX_DEVICE_NULL_H
#include "LuxEngine/LuxDevice.h"
#include "core/lxTimers.h"

namespace lux
{

class LuxDeviceNull : public LuxDevice {
public:
	LuxDeviceNull();
	~LuxDeviceNull();

	void BuildVideoDriver(const video::DriverConfig& config, void* user=nullptr) override;

	core::Array<core::String> GetVideoDriverTypes() override;
	StrongRef<video::AdapterList> GetVideoAdapters(const core::String& driver) override;

	void BuildImageSystem() override;
	void BuildScene(const core::String& renderer, void* user=nullptr) override;
	StrongRef<scene::Scene> CreateScene() override;
	void BuildGUIEnvironment() override;
	void BuildAll(const video::DriverConfig& config) override;

	StrongRef<scene::Scene> GetScene() const override;
	StrongRef<scene::SceneRenderer> GetSceneRenderer() const override;
	StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const override;

	void RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop) override;

protected:
	void ReleaseModules();
	StrongRef<scene::SceneRenderer> CreateSceneRenderer(const core::String& name, scene::Scene* scene, void* user);

protected:
	StrongRef<scene::Scene> m_Scene;
	StrongRef<scene::SceneRenderer> m_SceneRenderer;
	StrongRef<gui::GUIEnvironment> m_GUIEnv;
};

} // namespace lux

#endif // #ifndef INCLUDED_LUX_LUX_DEVICE_NULL_H