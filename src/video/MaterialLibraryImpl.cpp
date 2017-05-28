#include "MaterialLibraryImpl.h"

#include "video/Shader.h"
#include "video/MaterialRenderer.h"
#include "video/VideoDriver.h"

#include "io/FileSystem.h"
#include "io/File.h"

#include "core/lxAlgorithm.h"
#include "core/Logger.h"

namespace lux
{
namespace video
{

MaterialLibraryImpl::MaterialLibraryImpl(VideoDriver* driver, io::FileSystem* fileSys) :
	m_VideoDriver(driver),
	m_FileSystem(fileSys)
{
}

StrongRef<Material> MaterialLibraryImpl::CreateMaterial(const string& name)
{
	return CreateMaterial(GetMaterialRenderer(name));
}

StrongRef<Material> MaterialLibraryImpl::CreateMaterial(MaterialRenderer* renderer)
{
	if(renderer == nullptr)
		renderer = GetSolidRenderer();

	return renderer->CreateMaterial();
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::AddMaterialRenderer(MaterialRenderer* renderer)
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

StrongRef<MaterialRenderer> MaterialLibraryImpl::CloneMaterialRenderer(const string& name, const string& oldName)
{
	return CloneMaterialRenderer(name, GetMaterialRenderer(oldName));
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::CloneMaterialRenderer(const string& name, const MaterialRenderer* old)
{
	LX_CHECK_NULL_ARG(old);

	return AddMaterialRenderer(old->Clone(name));
}

void MaterialLibraryImpl::RemoveMaterialRenderer(MaterialRenderer* renderer)
{
	for(auto it = m_Renderers.First(); it != m_Renderers.End(); ++it) {
		if(*it == renderer) {
			m_Renderers.Erase(it);
			return;
		}
	}
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::AddShaderMaterialRenderer(
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

StrongRef<MaterialRenderer> MaterialLibraryImpl::AddShaderMaterialRenderer(
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
		PSFile = VSFile = m_FileSystem->OpenFile(VSPath);
	} else {
		PSFile = m_FileSystem->OpenFile(PSPath);
		VSFile = m_FileSystem->OpenFile(VSPath);
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
	StrongRef<video::Shader> shader = m_VideoDriver->CreateShader(
		language,
		vsCode, VSEntryPoint.Data(), VSFile->GetSize(), VSMajor, VSMinor,
		psCode, PSEntryPoint.Data(), PSFile->GetSize(), PSMajor, PSMinor,
		errorList);

	return AddShaderMaterialRenderer(shader, baseMaterial, name);
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::GetMaterialRenderer(size_t index) const
{
	return m_Renderers.At(index);
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::GetMaterialRenderer(const string& name) const
{
	size_t id;
	if(!FindRenderer(name, id))
		throw core::ObjectNotFoundException(name.Data());

	return m_Renderers[id];
}

size_t MaterialLibraryImpl::GetMaterialRendererCount() const
{
	return m_Renderers.Size();
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::GetSolidRenderer()
{
	if(!m_Solid)
		m_Solid = GetMaterialRenderer("solid");

	return m_Solid;
}

bool MaterialLibraryImpl::FindRenderer(const string& name, size_t& id) const
{
	for(id = 0; id < m_Renderers.Size(); ++id) {
		if(m_Renderers[id]->GetName() == name)
			return true;
	}

	return false;
}

} // !namespace video
} // !namespace lux
