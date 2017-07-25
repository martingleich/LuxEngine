#include "video/MaterialLibrary.h"

#include "video/Shader.h"
#include "video/VideoDriver.h"

#include "io/FileSystem.h"
#include "io/File.h"

#include "core/lxAlgorithm.h"
#include "core/Logger.h"

namespace lux
{
namespace video
{

static StrongRef<MaterialLibrary> g_MaterialLibrary;

void MaterialLibrary::Initialize(MaterialLibrary* matLib)
{
	if(!matLib)
		matLib = LUX_NEW(MaterialLibrary);

	if(!matLib)
		throw core::ErrorException("No material library available");
	g_MaterialLibrary = matLib;
}

MaterialLibrary::~MaterialLibrary()
{
	m_Solid.Reset();

	// At this point there shouldn't be any references to renderers
	for(auto& x : m_Renderers) {
		if(x->GetReferenceCount() != 1)
			log::Error("~MaterialLibrary still references to renderers");
	}
}

MaterialLibrary* MaterialLibrary::Instance()
{
	return g_MaterialLibrary;
}

void MaterialLibrary::Destroy()
{
	g_MaterialLibrary.Reset();
}

MaterialLibrary::MaterialLibrary()
{
	{
		AddMaterialRenderer("solidBase");
	}

	{
		auto transparentBase = AddMaterialRenderer("transparentBase");
		transparentBase->GetPass(0).isTransparent = true;
	}

	{
		m_Solid = AddMaterialRenderer("solid");
		m_Solid->GetPass(0).AddTexture();
		m_Solid->AddParam("diffMap", 0, (u32)EOptionId::Layer0);
	}

	{
		auto debug = AddMaterialRenderer("debugOverlay");
		auto& pass = debug->GetPass(0);
		pass.fogEnabled = false;
		pass.lighting = false;

		TextureStageSettings tss0;
		tss0.colorOperator = ETextureOperator::SelectArg1;
		tss0.colorArg1 = ETextureArgument::Diffuse;
		pass.layerSettings.PushBack(tss0);
		pass.useVertexColor = true;
	}

	{
		auto solidMix = AddMaterialRenderer("solidMix");
		auto& pass = solidMix->GetPass(0);
		pass.AddTexture(2);
		pass.useVertexColor = true;
		// Blend(Diff*Texture1, Texture2, DiffAlpha)
		TextureStageSettings tss0, tss1, tss2;
		tss0.colorOperator = ETextureOperator::SelectArg1;
		tss0.colorArg1 = ETextureArgument::Texture;

		tss1.colorOperator = ETextureOperator::Blend;
		tss1.colorArg1 = ETextureArgument::Current;
		tss1.colorArg2 = ETextureArgument::Texture;
		tss1.coordSource = 0;

		tss2.colorOperator = ETextureOperator::Modulate;
		tss2.colorArg1 = ETextureArgument::Current;
		tss2.colorArg2 = ETextureArgument::Diffuse;
		tss2.coordSource = 0;
		pass.layerSettings.PushBack(tss0);
		pass.layerSettings.PushBack(tss1);
		pass.layerSettings.PushBack(tss2);

		solidMix->AddParam("diffMap1", 0, (u32)EOptionId::Layer0);
		solidMix->AddParam("diffMap2", 0, (u32)EOptionId::Layer1);
	}

	{
		auto transparent = AddMaterialRenderer("transparent");
		auto& pass = transparent->GetPass(0);
		pass.alphaSrcBlend = video::EBlendFactor::SrcAlpha;
		pass.alphaDstBlend = video::EBlendFactor::OneMinusSrcAlpha;
		pass.alphaOperator = video::EBlendOperator::Add;
		pass.zWriteEnabled = false;
		pass.fogEnabled = false;
		pass.AddTexture();

		transparent->AddParam("diffMap", 0, (u32)EOptionId::Layer0);
	}
}

StrongRef<Material> MaterialLibrary::CreateMaterial(const String& name)
{
	return CreateMaterial(GetMaterialRenderer(name));
}

StrongRef<Material> MaterialLibrary::CreateMaterial(MaterialRenderer* renderer)
{
	if(renderer == nullptr)
		renderer = m_Solid;

	return renderer->CreateMaterial();
}

StrongRef<MaterialRenderer> MaterialLibrary::AddMaterialRenderer(MaterialRenderer* renderer)
{
	LX_CHECK_NULL_ARG(renderer);

	if(renderer->GetName().IsEmpty())
		throw core::InvalidArgumentException("renderer", "Renderer must have an name");

	size_t id;
	if(FindRenderer(renderer->GetName(), id))
		throw core::InvalidArgumentException("renderer", "Name is already used");

	m_Renderers.PushBack(renderer);

	return renderer;
}

StrongRef<MaterialRenderer> MaterialLibrary::AddMaterialRenderer(const String& newName)
{
	StrongRef<MaterialRenderer> r = LUX_NEW(MaterialRenderer)(newName);
	return AddMaterialRenderer(r);
}

StrongRef<MaterialRenderer> MaterialLibrary::CloneMaterialRenderer(const String& name, const String& oldName)
{
	return CloneMaterialRenderer(name, GetMaterialRenderer(oldName));
}

StrongRef<MaterialRenderer> MaterialLibrary::CloneMaterialRenderer(const String& name, const MaterialRenderer* old)
{
	LX_CHECK_NULL_ARG(old);

	auto r = LUX_NEW(MaterialRenderer)(name, old);
	return AddMaterialRenderer(r);
}

void MaterialLibrary::RemoveMaterialRenderer(MaterialRenderer* renderer)
{
	if(!renderer)
		return;

	for(auto it = m_Renderers.First(); it != m_Renderers.End(); ++it) {
		if(*it == renderer) {
			m_Renderers.Erase(it);
			return;
		}
	}
}

StrongRef<Shader> MaterialLibrary::CreateShaderFromFile(
	video::EShaderLanguage language,
	const io::Path& VSPath, const String& VSEntryPoint, int VSMajor, int VSMinor,
	const io::Path& PSPath, const String& PSEntryPoint, int PSMajor, int PSMinor,
	core::Array<String>* errorList)
{
	StrongRef<io::File> PSFile;
	StrongRef<io::File> VSFile;
	if(VSPath == PSPath) {
		PSFile = VSFile = io::FileSystem::Instance()->OpenFile(VSPath);
	} else {
		PSFile = io::FileSystem::Instance()->OpenFile(PSPath);
		VSFile = io::FileSystem::Instance()->OpenFile(VSPath);
	}

	core::mem::RawMemory vsCodeBuffer;
	char* vsCode = (char*)VSFile->GetBuffer();
	if(!vsCode) {
		vsCodeBuffer.SetSize(VSFile->GetSize() + 1);
		vsCode = vsCodeBuffer;
		VSFile->ReadBinary(VSFile->GetSize(), vsCode);
		vsCode[VSFile->GetSize()] = 0;
	}

	core::mem::RawMemory psCodeBuffer;
	char* psCode = (char*)PSFile->GetBuffer();
	if(!psCode) {
		if(PSFile == VSFile) {
			psCode = vsCode;
		} else {
			psCodeBuffer.SetSize(PSFile->GetSize() + 1);
			PSFile->ReadBinary(PSFile->GetSize(), psCode);
			psCode[PSFile->GetSize()] = 0;
		}
	}

	return video::VideoDriver::Instance()->CreateShader(
		language,
		vsCode, VSEntryPoint.Data(), VSFile->GetSize(), VSMajor, VSMinor,
		psCode, PSEntryPoint.Data(), PSFile->GetSize(), PSMajor, PSMinor,
		errorList);
}

StrongRef<MaterialRenderer> MaterialLibrary::GetMaterialRenderer(size_t index) const
{
	return m_Renderers.At(index);
}

StrongRef<MaterialRenderer> MaterialLibrary::GetMaterialRenderer(const String& name) const
{
	size_t id;
	if(!FindRenderer(name, id))
		throw core::ObjectNotFoundException(name.Data());

	return m_Renderers[id];
}

bool MaterialLibrary::ExistsMaterialRenderer(const String& name, MaterialRenderer** outRenderer) const
{
	size_t id;
	if(!FindRenderer(name, id))
		return false;

	if(outRenderer)
		*outRenderer = m_Renderers[id];
	return true;
}

size_t MaterialLibrary::GetMaterialRendererCount() const
{
	return m_Renderers.Size();
}

bool MaterialLibrary::FindRenderer(const String& name, size_t& id) const
{
	for(id = 0; id < m_Renderers.Size(); ++id) {
		if(m_Renderers[id]->GetName() == name)
			return true;
	}

	return false;
}

StrongRef<Shader> MaterialLibrary::CreateShaderFromMemory(
	EShaderLanguage language,
	const String& VSCode, const char* VSEntryPoint, int VSmajorVersion, int VSminorVersion,
	const String& PSCode, const char* PSEntryPoint, int PSmajorVersion, int PSminorVersion,
	core::Array<String>* errorList)
{
	return video::VideoDriver::Instance()->CreateShader(
		language,
		VSCode.Data_c(), VSEntryPoint, VSCode.Size(), VSmajorVersion, VSminorVersion,
		PSCode.Data_c(), PSEntryPoint, PSCode.Size(), PSmajorVersion, PSminorVersion,
		errorList);
}

} // namespace video
} // namespace lux
