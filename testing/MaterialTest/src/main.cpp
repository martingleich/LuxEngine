#include "Context.h"

using namespace lux;

AppContext Context;

class MaterialTest
{
private:
	StrongRef<LuxDevice> m_Device;

public:
	MaterialTest()
	{
		log::EngineLog.SetLogLevel(log::ELogLevel::Info);
		log::EngineLog.SetNewPrinter(log::ConsolePrinter);

		m_Device = CreateDevice();
		auto config = video::DriverConfig::WindowedDefault(800, 600);
		config.multiSampling = 10;
		m_Device->BuildAll(config);

		Context = AppContext(m_Device);

		Context.Input->GetEventSignal().Connect(this, &MaterialTest::OnEvent);
	}

	void Move(float secsPassed)
	{
		m_Time += secsPassed;
		Context.Renderer->GetParam("time") = m_Time;
	}

	void Render();

	void DoFrame(float secsPassed)
	{
		if(Context.Window->IsActive()) {
			Context.Smgr->AnimateAll(secsPassed);
			Move(secsPassed);

			if(Context.Smgr->DrawAll(true, false)) {
				Render();
				Context.Renderer->EndScene();
				Context.Renderer->Present();
			}

			if(Context.Input->GetKeyboard()->GetButton(input::KEY_ESCAPE)->state)
				Context.Device->CloseDevice();
		}
	}

	void LoadBase()
	{
		scene::Node* node;

		Context.CursorCtrl->Disable();

		m_Camera = Context.Smgr->CreateCamera();
		m_CameraNode = Context.Smgr->AddNode(m_Camera);
		m_CameraNode->SetPosition(0.0f, 0.0f, -15.0f);
		m_CameraNode->SetDirectionUp(math::Vector3F(0.0f, 0.0f, 1.0f));

		m_Light = Context.Smgr->CreateLight();
		m_Light->SetLightType(video::ELightType::Directional);
		m_Light->SetColor(video::Color::White);
		node = Context.Smgr->AddNode(m_Light);
		node->SetDirection(math::Vector3F::UNIT_Z);

		node = Context.Smgr->AddSkyBox();
		node->GetComponent<scene::SkyBox>()->GetMaterial(0)->SetDiffuse(video::Color::Blue);
	}

	void Run()
	{
		Load();

		float secsPassed;
		while(Context.Device->Run(secsPassed))
			DoFrame(secsPassed);
	}

	void Load();

	void OnEvent(const input::Event& e);

	core::Array<StrongRef<video::Material>> GenMaterialList();

private:
	StrongRef<scene::Node> m_CameraNode;
	StrongRef<scene::Camera> m_Camera;
	StrongRef<scene::Light> m_Light;
	StrongRef<scene::SkyBox> m_SkyBox;

	StrongRef<gui::Font> m_Font;

	float m_Time = 0.0f;
};

void MaterialTest::Render()
{
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

	auto mat = Context.MatLib->CreateMaterial("solid");
	mat->SetDiffuse(video::Color::Red);
	out.PushBack(mat);

	return out;
}

void MaterialTest::Load()
{
	LoadBase();

	m_Font = Context.GUI->GetFontCreator()->CreateFont(
		gui::FontDescription("Times New Roman", 40),
		Context.GUI->GetFontCreator()->GetDefaultCharset("german"));

	auto materials = GenMaterialList();

	const math::Vector3F startPoint = math::Vector3F(-8.0f, 6.0f, 0.0f);
	const float delta = 3.0f;

	StrongRef<video::Mesh> geo = Context.MeshSystem->CreateSphereMesh();

	size_t row = 0;
	size_t col = 0;
	for(auto it = materials.First(); it != materials.End(); ++it) {
		auto comp = Context.Smgr->CreateMesh(geo);
		comp->SetReadMaterialsOnly(false);
		comp->SetMaterial(0, *it);

		auto node = Context.Smgr->AddNode(comp);

		math::Vector3F pos(
			startPoint.x + col*delta,
			startPoint.y + row*delta,
			0.0f);
		node->SetPosition(pos);
	}
}

void MaterialTest::OnEvent(const input::Event& e)
{
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

