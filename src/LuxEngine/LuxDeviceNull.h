#ifndef INCLUDED_LUX_DEVICE_NULL_H
#define INCLUDED_LUX_DEVICE_NULL_H
#include "LuxEngine/LuxDevice.h"

namespace lux
{

class LuxDeviceNull : public LuxDevice
{
public:
	LuxDeviceNull();
	~LuxDeviceNull();

	void ReleaseModules();

	void BuildVideoDriver(const video::DriverConfig& config, void* user=nullptr);

	core::Array<core::String> GetDriverTypes();
	StrongRef<video::AdapterList> GetVideoAdapters(const core::String& driver);

	void BuildImageSystem();
	void BuildScene(const core::String& renderer, void* user=nullptr);
	StrongRef<scene::Scene> CreateScene();
	StrongRef<scene::SceneRenderer> CreateSceneRenderer(const core::String& name, scene::Scene* scene, void* user=nullptr);
	void BuildGUIEnvironment();
	void BuildAll(const video::DriverConfig& config);

	StrongRef<scene::Scene> GetScene() const;
	StrongRef<scene::SceneRenderer> GetSceneRenderer() const;
	StrongRef<gui::GUIEnvironment> GetGUIEnvironment() const;

	void RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop);

protected:
	StrongRef<scene::Scene> m_Scene;
	StrongRef<scene::SceneRenderer> m_SceneRenderer;
	StrongRef<gui::GUIEnvironment> m_GUIEnv;
};

} // namespace lux

#endif // #ifndef INCLUDED_LUX_DEVICE_NULL_H