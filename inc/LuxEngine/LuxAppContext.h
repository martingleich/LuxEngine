#ifndef INCLUDED_LUX_APP_CONTEXT_H
#define INCLUDED_LUX_APP_CONTEXT_H
#include "LuxEngine/LuxDevice.h"
#include "video/VideoDriver.h"
#include "video/images/ImageSystem.h"
#include "video/MaterialLibrary.h"
#include "video/mesh/MeshSystem.h"
#include "input/InputSystem.h"
#include "io/FileSystem.h"
#include "gui/GUIEnvironment.h"
#include "core/ReferableFactory.h"
#include "core/ResourceSystem.h"

namespace lux
{

struct LuxAppContext
{
	LuxAppContext()
	{
	}

	LuxAppContext(LuxDevice* device)
	{
		if(!device)
			return;

		Device = device;

		Driver = video::VideoDriver::Instance();
		ImgSys = video::ImageSystem::Instance();
		MatLib = video::MaterialLibrary::Instance();
		MeshSystem = video::MeshSystem::Instance();

		Scene = Device->GetScene();
		SceneRenderer = Device->GetSceneRenderer();
		GUI = Device->GetGUIEnvironment();

		Input = input::InputSystem::Instance();

		FileSys = io::FileSystem::Instance();

		Window = Device->GetWindow();
		if(GUI)
			Cursor = GUI->GetCursor();
		else
			Cursor = Device->GetCursor();

		ResourceSystem = core::ResourceSystem::Instance();
		RefFactory = core::ReferableFactory::Instance();

		if(Driver)
			DriverConfig = Driver->GetConfig();

		if(Driver)
			Renderer = Driver->GetRenderer();
		else
			Renderer = nullptr;
	}

	// Don't save StrongReferences here.
	/*
	This structure is most often used globally and doesn't really own all these objects.
	*/
	LuxDevice* Device = nullptr;
	video::VideoDriver* Driver = nullptr;
	video::Renderer* Renderer = nullptr;
	scene::Scene* Scene = nullptr;
	scene::SceneRenderer* SceneRenderer = nullptr;
	input::InputSystem* Input = nullptr;
	io::FileSystem* FileSys = nullptr;
	video::MaterialLibrary* MatLib = nullptr;
	video::ImageSystem* ImgSys = nullptr;
	gui::Window* Window = nullptr;
	gui::GUIEnvironment* GUI = nullptr;
	core::ResourceSystem* ResourceSystem = nullptr;
	video::MeshSystem* MeshSystem = nullptr;
	video::DriverConfig DriverConfig;
	gui::Cursor* Cursor = nullptr;
	core::ReferableFactory* RefFactory = nullptr;

	StrongRef<video::Mesh> GetMesh(core::StringView path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Mesh, path).As<video::Mesh>();
	}

	StrongRef<video::Texture> GetTexture(core::StringView path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Texture, path).As<video::Texture>();
	}

	StrongRef<video::CubeTexture> GetCubeTexture(core::StringView path)
	{
		return ResourceSystem->GetResource(core::ResourceType::CubeTexture, path).As<video::CubeTexture>();
	}

	StrongRef<video::Image> GetImage(core::StringView path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Image, path).As<video::Image>();
	}
};

} // namespace lux

#endif // #ifndef INCLUDED_LUX_APP_CONTEXT_H
