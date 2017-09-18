#include "Context.h"

using namespace lux;

AppContext Context;

class MaterialTest : public input::EventHandler
{
private:
	StrongRef<LuxDevice> m_Device;

public:
	MaterialTest()
	{
		log::EngineLog.SetLogLevel(log::ELogLevel::Info);
		log::EngineLog.SetNewPrinter(log::ConsolePrinter);

		m_Device = CreateDevice();
		auto adapter = m_Device->GetVideoAdapters(video::DriverType::Direct3D9)->GetAdapter(0);
		video::DriverConfig config;
		adapter->GenerateConfig(config,
			math::Dimension2U(800, 600),
			true, true,
			false,
			0,
			8,
			5);
		m_Device->BuildAll(config);

		Context = AppContext(m_Device);

		Context.Input->GetEventSignal().Connect(this, &MaterialTest::OnEvent);
	}

	void Render(float secsPassed);

	void LoadBase()
	{
		scene::Node* node;

		m_Camera = Context.Scene->CreateCamera();
		m_CameraNode = Context.Scene->AddNode(m_Camera);
		m_CameraNode->SetPosition(0.0f, 0.0f, -15.0f);
		m_CameraNode->SetDirectionUp(math::Vector3F(0.0f, 0.0f, 1.0f));

		m_Light = Context.Scene->CreateLight();
		m_Light->SetLightType(video::ELightType::Directional);
		m_Light->SetColor(video::Color::White);
		node = Context.Scene->AddNode(m_Light);
		node->SetDirection(math::Vector3F::UNIT_Z);

		m_CheckerTexture = Context.ImgSys->CreateFittingTexture(math::Dimension2U(32, 32));
		m_CheckerTexture->SetFiltering(video::BaseTexture::Filter::Point);
		{
			auto canvas = std::move(m_CheckerTexture->GetCanvas(video::BaseTexture::ELockMode::Overwrite));
			bool black = false;
			core::Randomizer rand(123456);
			for(size_t r = 0; r < canvas.GetHeight(); ++r) {
				for(size_t c = 0; c < canvas.GetWidth(); ++c) {
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

		node = Context.Scene->AddSkyBox();
		auto skyBox = node->GetComponent<scene::SkyBox>();
		skyBox->UseCubeTexture(false);
		skyBox->SetSkyTexture(m_CheckerTexture);
	}

	void Run()
	{
		Load();

		LuxDevice::SimpleFrameLoop floop;
		floop.userData = this;
		floop.postRenderProc = [](float secsPassed, void* user) {
			reinterpret_cast<MaterialTest*>(user)->Render(secsPassed);
		};
		Context.Device->RunSimpleFrameLoop(floop);
	}

	void Load();

	void OnEvent(const input::Event& e)
	{
		if(m_RButton || e.source == input::EEventSource::Keyboard)
			m_CameraNode->GetComponent<scene::FirstPersonCameraControl>()->DefaultEventToCameraAction(e);
		EventHandler::OnEvent(e);
	}

	void OnKey(bool isDown, input::EKeyCode key, const input::Event& event)
	{
		if(isDown) {
			switch(key) {
			case input::EKeyCode::KEY_ESCAPE:
				Context.Window->Close();
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

	StrongRef<gui::Font> m_Font;

	StrongRef<video::Texture> m_CheckerTexture;

	float m_Time = 0.0f;
};

void MaterialTest::Render(float secsPassed)
{
	m_Time += secsPassed;
	if(fmodf(m_Time, 2.0f) < 1.0f) {
		m_Font->Draw("Blinking Text",
			math::Vector2F(0.0f, 50.0f),
			gui::Font::EAlign::BottomLeft,
			video::Color::Green);
	}
}

core::Array<StrongRef<video::Material>> MaterialTest::GenMaterialList()
{
	core::Array<StrongRef<video::Material>> out;
	StrongRef<video::Material> mat;

	mat = Context.MatLib->CreateMaterial("solid");
	mat->SetDiffuse(video::Color::Red);
	out.PushBack(mat);

	mat = Context.MatLib->CreateMaterial("transparent");
	mat->SetDiffuse(video::Color::Green);
	mat->SetAlpha(0.5f);
	out.PushBack(mat);

	return out;
}

void MaterialTest::Load()
{
	LoadBase();

	StrongRef<scene::Node> node;

	m_CameraNode->AddComponent(Context.Scene->CreateFirstPersonCameraControl());

	m_Font = Context.GUI->GetFontCreator()->CreateFont(
		gui::FontDescription("Comic Sans MS", 40, gui::EFontWeight::Bolt),
		Context.GUI->GetFontCreator()->GetDefaultCharset("german"));

	auto materials = GenMaterialList();

	const math::Vector3F startPoint = math::Vector3F(-8.0f, 6.0f, 0.0f);
	const float delta = 3.0f;

	StrongRef<video::Mesh> geo = Context.MeshSystem->CreateSphereMesh();

	size_t row = 0;
	size_t col = 0;
	for(auto it = materials.First(); it != materials.End(); ++it) {
		auto comp = Context.Scene->CreateMesh(geo);
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
