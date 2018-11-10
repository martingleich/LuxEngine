#ifndef INCLUDED_LUX_LUX_DEVICE_NULL_H
#define INCLUDED_LUX_LUX_DEVICE_NULL_H
#include "LuxEngine/LuxDevice.h"
#include "core/lxTimers.h"

namespace lux
{

class LuxDeviceNull : public LuxDevice
{
public:
	LuxDeviceNull();
	~LuxDeviceNull();

	core::Array<core::Name> GetVideoDriverTypes() override;
	StrongRef<video::AdapterList> GetVideoAdapters(core::Name driver) override;
	void BuildVideoDriver(const video::DriverConfig& config) override;

	StrongRef<scene::SceneRenderer> CreateSceneRenderer(core::Name name, scene::Scene* scene) override;
	StrongRef<scene::Scene> CreateScene() override;

	void BuildImageSystem() override;
	void BuildGUIEnvironment() override;
	void BuildAll(const video::DriverConfig& config) override;

	void RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop) override;

	void AddSceneRenderer(core::Name name, scene::SceneRenderer* (*createFunc)(const scene::SceneRendererInitData&))
	{
		m_SceneRenderers[name] = SceneRendererEntry(createFunc);
	}

protected:
	void ReleaseModules();

	struct VideoDriverEntry
	{
		video::VideoDriver* (*driverCreateFunc)(const video::VideoDriverInitData&);
		video::AdapterList* (*adapterListCreateFunc)();
		VideoDriverEntry()
		{}
		VideoDriverEntry(
			video::VideoDriver* (*_driverCreateFunc)(const video::VideoDriverInitData&),
			video::AdapterList* (*_adapterListCreateFunc)()) :
			driverCreateFunc(_driverCreateFunc),
			adapterListCreateFunc(_adapterListCreateFunc)
		{}
	};

	struct SceneRendererEntry
	{
		scene::SceneRenderer* (*sceneRendererCreateFunc)(const scene::SceneRendererInitData&);
		SceneRendererEntry() 
		{}
		explicit SceneRendererEntry(
		scene::SceneRenderer* (*_sceneRendererCreateFunc)(const scene::SceneRendererInitData&))  :
			sceneRendererCreateFunc(_sceneRendererCreateFunc)
		{}
	};

protected:
	core::HashMap<core::Name, VideoDriverEntry> m_VideoDrivers;
	core::HashMap<core::Name, SceneRendererEntry> m_SceneRenderers;
};

} // namespace lux

#endif // #ifndef INCLUDED_LUX_LUX_DEVICE_NULL_H