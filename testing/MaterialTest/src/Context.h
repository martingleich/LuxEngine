#ifndef INCLUDED_CONTEXT_H
#define INCLUDED_CONTEXT_H
#include <Lux.h>

using namespace lux;

struct AppContext
{
	AppContext() : Device(nullptr), Driver(nullptr), Smgr(nullptr), Input(nullptr)
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

		Smgr = Device->GetSceneManager();
		GUI = Device->GetGUIEnvironment();

		Input = input::InputSystem::Instance();

		FileSys = io::FileSystem::Instance();

		Window = Device->GetWindow();
		CursorCtrl = Window->GetCursorControl();

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
			MixRenderer = MatLib->GetMaterialRenderer("solid_mix");
		} else {
			SolidRenderer = nullptr;
			TransparentRenderer = nullptr;
			MixRenderer = nullptr;
		}
	}

	LuxDevice* Device;
	video::VideoDriver* Driver;
	video::Renderer* Renderer;
	scene::SceneManager* Smgr;
	input::InputSystem* Input;
	io::FileSystem* FileSys;
	video::MaterialLibrary* MatLib;
	video::ImageSystem* ImgSys;
	gui::Window* Window;
	gui::GUIEnvironment* GUI;
	core::ResourceSystem* ResourceSystem;
	video::MeshSystem* MeshSystem;
	video::DriverConfig Config;
	gui::CursorControl* CursorCtrl;
	core::ReferableFactory* RefFactory;

	video::MaterialRenderer* SolidRenderer;
	video::MaterialRenderer* TransparentRenderer;
	video::MaterialRenderer* MixRenderer;

	StrongRef<scene::Mesh> GetMesh(const io::path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Mesh, path).As<scene::Mesh>();
	}

	StrongRef<video::Texture> GetTexture(const io::path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Texture, path).As<video::Texture>();
	}

	StrongRef<video::CubeTexture> GetCubeTexture(const io::path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::CubeTexture, path).As<video::CubeTexture>();
	}

	StrongRef<video::Image> GetImage(const io::path& path)
	{
		return ResourceSystem->GetResource(core::ResourceType::Image, path).As<video::Image>();
	}

	StrongRef<video::MaterialRenderer> CreateHLSLShader(const io::path& path, bool isSolid=true)
	{
		core::array<string> errorList;
		StrongRef<video::MaterialRenderer> shader;
		try {
			shader = MatLib->AddShaderMaterialRenderer(video::EShaderLanguage::HLSL,
				path, "mainVS", 2, 0,
				path, "mainPS", 2, 0,
				MatLib->GetMaterialRenderer(isSolid?"solid_base":"transparent_base"),
				path, &errorList);
		} catch(video::ShaderCompileException&) {
			for(auto it = errorList.First(); it != errorList.End(); ++it)
				log::Error(*it);
			throw;
		}

		return shader;
	}
};

#endif // #ifndef INCLUDED_CONTEXT_H