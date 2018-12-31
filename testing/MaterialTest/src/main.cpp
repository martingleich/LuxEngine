#include <Lux.h>
#include <LuxRebuiltMarker.h>
#include "scene/components/Light.h"
#include "scene/components/SkyBox.h"
#include "scene/components/Camera.h"
#include "scene/components/SceneMesh.h"
#include "scene/components/FirstPersonCameraControl.h"
#include "scene/SceneBuilder.h"

#include "gui/elements/GUIStaticText.h"

using namespace lux;

LuxAppContext Context;

class MaterialTest : public input::EventHandler, public LuxDevice::SimpleFrameLoopCallback
{
private:
	StrongRef<LuxDevice> m_Device;
	StrongRef<scene::Scene> m_Scene;
	StrongRef<scene::SceneRenderer> m_SceneRenderer;

public:
	MaterialTest()
	{
		log::SetLogLevel(log::ELogLevel::Info);
		log::SetPrinter(log::ConsolePrinter);

		m_Device = CreateDevice();
		auto adapter = m_Device->GetVideoAdapters(video::DriverType::Direct3D9)->GetAdapter(0);
		video::DriverConfig config;
		adapter->GenerateConfig(config,
			math::Dimension2I(800, 600),
			true, true,
			0,
			8,
			5);
		m_Device->BuildAll(config);
		m_Scene = m_Device->CreateScene();
		m_SceneRenderer = m_Device->CreateSceneRenderer(core::Name("SimpleForward"), m_Scene);

		Context = LuxAppContext(m_Device, m_Scene, m_SceneRenderer);

		Context.Input->GetEventSignal().Connect(this, &MaterialTest::OnEvent);
	}

	void LoadBase()
	{
		scene::Node* node;
		scene::SceneBuilder sceneBuilder(Context.Scene);

		m_Camera = sceneBuilder.CreateCamera();
		m_CameraNode = Context.Scene->AddNode(m_Camera);
		m_CameraNode->SetPosition(0.0f, 0.0f, -15.0f);
		m_CameraNode->SetDirectionUp(math::Vector3F(0.0f, 0.0f, 1.0f));

		m_Light = sceneBuilder.CreateLight();
		m_Light->SetLightType(video::ELightType::Directional);
		m_Light->SetColor(video::Color::White);
		node = sceneBuilder.AddNode(m_Light);
		node->SetDirection(math::Vector3F::UNIT_Z);

		m_CheckerTexture = Context.ImgSys->CreateFittingTexture(math::Dimension2I(32, 32));
		m_CheckerTexture->SetFiltering(video::BaseTexture::Filter::Point);
		{
			auto canvas = std::move(m_CheckerTexture->GetCanvas(video::BaseTexture::ELockMode::Overwrite));
			bool black = false;
			core::Randomizer rand(123456);
			for(int r = 0; r < canvas.GetHeight(); ++r) {
				for(int c = 0; c < canvas.GetWidth(); ++c) {
					black = ((r + c) % 2 == 0);
					if(rand.GetBool(0.1f)) {
						canvas.SetPixel(c, r, video::HSVToColorf(rand.GetFloat(0, 1), 1.0f, 1.0f).ToHex());
					} else {
						if(black)
							canvas.SetPixel(c, r, video::Color::Black);
						else
							canvas.SetPixel(c, r, video::Color::White);
					}
				}
			}
		}

		node = sceneBuilder.AddSkyBox();
		auto skyBox = node->GetComponent<scene::SkyBox>();
		skyBox->UseCubeTexture(false);
		skyBox->SetSkyTexture(m_CheckerTexture);
	}

	void Run()
	{
		Load();

		LuxDevice::SimpleFrameLoop floop;
		floop.callback = this;
		floop.scene = m_Scene;
		floop.sceneRenderer = m_SceneRenderer;
		Context.Device->RunSimpleFrameLoop(floop);
	}

	void Load();

	void OnEvent(const input::Event& e)
	{
		if(m_RButton || e.source == input::EEventSource::Keyboard)
			m_CameraNode->GetComponent<scene::FirstPersonCameraControl>()->HandleInput(e);
		EventHandler::OnEvent(e);
	}

	void OnKey(bool isDown, input::EKeyCode key, const input::Event& event)
	{
		if(isDown) {
			switch(key) {
			case input::EKeyCode::KEY_ESCAPE:
				Context.Window->Close();
				break;
			default:
				break;
			}
		}
	}
	void OnRButton(bool isDown, const input::Event& event)
	{
		m_RButton = isDown;
		if(isDown)
			Context.Cursor->Disable();
		else
			Context.Cursor->Enable();
	}

	core::Array<StrongRef<video::Material>> GenMaterialList();

private:
	bool m_RButton = false;

	StrongRef<scene::Node> m_CameraNode;
	StrongRef<scene::Camera> m_Camera;
	StrongRef<scene::Light> m_Light;
	StrongRef<scene::SkyBox> m_SkyBox;

	StrongRef<video::Texture> m_CheckerTexture;
};

core::Array<StrongRef<video::Material>> MaterialTest::GenMaterialList()
{
	core::Array<StrongRef<video::Material>> out;
	StrongRef<video::Material> mat;

	mat = Context.MatLib->CloneMaterial("solid");
	mat->SetDiffuse(video::Color::Red);
	out.PushBack(mat);

	mat = Context.MatLib->CloneMaterial("transparent");
	mat->SetDiffuse(video::Color::Green);
	mat->SetAlpha(0.5f);
	out.PushBack(mat);

	return out;
}

void MaterialTest::Load()
{
	LoadBase();

	StrongRef<scene::Node> node;
	scene::SceneBuilder scenebuilder(Context.Scene);

	m_CameraNode->AddComponent(scenebuilder.CreateFirstPersonCameraControl());

	auto font = Context.GUI->GetFontCreator()->CreateFont(
		gui::FontDescription("Comic Sans MS", 40, gui::EFontWeight::Bolt),
		Context.GUI->GetFontCreator()->GetDefaultCharset("german"));
	auto text = Context.GUI->AddStaticText(gui::PixelVector(0, 50), "Test Text");
	text->SetTextColor(video::Color::Green);
	text->SetFont(font);

	auto materials = GenMaterialList();

	const math::Vector3F startPoint = math::Vector3F(-8.0f, 6.0f, 0.0f);
	const float delta = 3.0f;

	StrongRef<video::Mesh> geo = Context.MeshSystem->CreateSphereMesh();

	size_t row = 0;
	size_t col = 0;
	for(auto it = materials.First(); it != materials.End(); ++it) {
		auto comp = scenebuilder.CreateMesh(geo);
		comp->SetReadMaterialsOnly(false);
		comp->SetMaterial(0, *it);

		node = Context.Scene->AddNode(comp);

		math::Vector3F pos(
			startPoint.x + col*delta,
			startPoint.y + row*delta,
			0.0f);
		node->SetPosition(pos);

		col += 1;
		if(col == 6) {
			++row;
			col = 0;
		}
	}
}

int main(int arg, char **argv)
{
	{
		MaterialTest mt;
		mt.Run();
	}

	getchar();
	return 0;
}
