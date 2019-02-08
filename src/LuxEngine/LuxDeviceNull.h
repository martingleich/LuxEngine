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

	StrongRef<scene::Scene> CreateScene() override;

	void BuildVideoDriver(const video::DriverConfig& config) override;
	// Canvas3DSystem, ShaderFactory
	void BuildVideoDriverHelpers() override;

	void BuildImageSystem() override;
	void BuildGUIEnvironment() override;

	void BuildMaterialLibrary() override;
	void BuildMeshSystem(video::Material* defaultMaterial) override;

	void RunSimpleFrameLoop(const SimpleFrameLoop& frameLoop) override;

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

protected:
	core::HashMap<core::Name, VideoDriverEntry> m_VideoDrivers;
};

} // namespace lux

#endif // #ifndef INCLUDED_LUX_LUX_DEVICE_NULL_H