#include "video/MaterialLibrary.h"

#include "video/Shader.h"
#include "video/MaterialRendererImpl.h"
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
	AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_BaseSolid)("solid_base", nullptr, nullptr));
	AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_BaseTransparent)("transparent_base", nullptr, nullptr));
	m_Solid = AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_Solid)("solid", nullptr, nullptr));
	AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_Solid_Mix)("solid_mix", nullptr, nullptr));
	AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_OneTextureBlend)("transparent", nullptr, nullptr));
	AddMaterialRenderer(LUX_NEW(video::MaterialRenderer_DebugOverlay)("debug_overlay", nullptr, nullptr));
	//AddMaterialRenderer(LUX_NEW(video::CMaterialRenderer_VertexAlpha_d3d9)(nullptr, nullptr), "transparent_alpha");
}

StrongRef<Material> MaterialLibrary::CreateMaterial(const string& name)
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

StrongRef<MaterialRenderer> MaterialLibrary::CloneMaterialRenderer(const string& name, const string& oldName)
{
	return CloneMaterialRenderer(name, GetMaterialRenderer(oldName));
}

StrongRef<MaterialRenderer> MaterialLibrary::CloneMaterialRenderer(const string& name, const MaterialRenderer* old)
{
	LX_CHECK_NULL_ARG(old);

	return AddMaterialRenderer(old->Clone(name));
}

void MaterialLibrary::RemoveMaterialRenderer(MaterialRenderer* renderer)
{
	for(auto it = m_Renderers.First(); it != m_Renderers.End(); ++it) {
		if(*it == renderer) {
			m_Renderers.Erase(it);
			return;
		}
	}
}

StrongRef<MaterialRenderer> MaterialLibrary::AddShaderMaterialRenderer(
	Shader* shader,
	const MaterialRenderer* baseMaterial, const string& name)
{
	LX_CHECK_NULL_ARG(shader);
	LX_CHECK_NULL_ARG(baseMaterial);

	if(baseMaterial->GetShader() != nullptr)
		throw core::InvalidArgumentException("baseMaterial", "Base Material must not have a shader");

	StrongRef<video::MaterialRenderer> renderer;
	if(shader)
		renderer = baseMaterial->Clone(name, shader, &shader->GetParamPackage());

	return AddMaterialRenderer(renderer);
}

StrongRef<MaterialRenderer> MaterialLibrary::AddShaderMaterialRenderer(
	video::EShaderLanguage language,
	const io::path& VSPath, const string& VSEntryPoint, int VSMajor, int VSMinor,
	const io::path& PSPath, const string& PSEntryPoint, int PSMajor, int PSMinor,
	const MaterialRenderer* baseMaterial, const string& name,
	core::array<string>* errorList)
{
	LX_CHECK_NULL_ARG(baseMaterial);

	if(baseMaterial->GetShader() != nullptr)
		throw core::InvalidArgumentException("baseMaterial", "Base Material must not have a shader");

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
	StrongRef<video::Shader> shader = video::VideoDriver::Instance()->CreateShader(
		language,
		vsCode, VSEntryPoint.Data(), VSFile->GetSize(), VSMajor, VSMinor,
		psCode, PSEntryPoint.Data(), PSFile->GetSize(), PSMajor, PSMinor,
		errorList);

	return AddShaderMaterialRenderer(shader, baseMaterial, name);
}

StrongRef<MaterialRenderer> MaterialLibrary::GetMaterialRenderer(size_t index) const
{
	return m_Renderers.At(index);
}

StrongRef<MaterialRenderer> MaterialLibrary::GetMaterialRenderer(const string& name) const
{
	size_t id;
	if(!FindRenderer(name, id))
		throw core::ObjectNotFoundException(name.Data());

	return m_Renderers[id];
}

size_t MaterialLibrary::GetMaterialRendererCount() const
{
	return m_Renderers.Size();
}

bool MaterialLibrary::FindRenderer(const string& name, size_t& id) const
{
	for(id = 0; id < m_Renderers.Size(); ++id) {
		if(m_Renderers[id]->GetName() == name)
			return true;
	}

	return false;
}
}
}
