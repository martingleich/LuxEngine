#ifndef INCLUDED_CONTEXT_H
#define INCLUDED_CONTEXT_H
#include <Lux.h>

using namespace lux;

struct AppContext
{
	AppContext() : Device(nullptr), Driver(nullptr), Scene(nullptr), Input(nullptr)
	{
	}

	AppContext(LuxDevice* pDevice)
	{
		if(!pDevice)
			return;

		Device = pDevice;

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
		Cursor = GUI->GetCursor();

		ResourceSystem = core::ResourceSystem::Instance();
		RefFactory = core::ReferableFactory::Instance();

		if(Driver)
			Config = Driver->GetConfig();

		if(Driver)
			Renderer = Driver->GetRenderer();
		else
			Renderer = nullptr;

		if(MatLib) {
			SolidRenderer = MatLib->GetMaterialRenderer("solid");
			TransparentRenderer = MatLib->GetMaterialRenderer("transparent");
			MixRenderer = MatLib->GetMaterialRenderer("solidMix");
		} else {
			SolidRenderer = nullptr;
			TransparentRenderer = nullptr;
			MixRenderer = nullptr;
		}
	}

	LuxDevice* Device;
	video::VideoDriver* Driver;
	video::Renderer* Renderer;
	scene::Scene* Scene;
	scene::SceneRenderer* SceneRenderer;
	input::InputSystem* Input;
	io::FileSystem* FileSys;
	video::MaterialLibrary* MatLib;
	video::ImageSystem* ImgSys;
	gui::Window* Window;
	gui::GUIEnvironment* GUI;
	core::ResourceSystem* ResourceSystem;
	video::MeshSystem* MeshSystem;
	video::DriverConfig Config;
	gui::Cursor* Cursor;
	core::ReferableFactory* RefFactory;

	video::MaterialRenderer* SolidRenderer;
	video::MaterialRenderer* TransparentRenderer;
	video::MaterialRenderer* MixRenderer;

	StrongRef<scene::Mesh> GetMesh(const io::Path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Mesh, path).As<scene::Mesh>();
	}

	StrongRef<video::Texture> GetTexture(const io::Path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Texture, path).As<video::Texture>();
	}

	StrongRef<video::CubeTexture> GetCubeTexture(const io::Path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::CubeTexture, path).As<video::CubeTexture>();
	}

	StrongRef<video::Image> GetImage(const io::Path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Image, path).As<video::Image>();
	}
};

#endif // #ifndef INCLUDED_CONTEXT_H