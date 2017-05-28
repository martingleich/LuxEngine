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

size_t MaterialLibraryImpl::AddMaterialRenderer(MaterialRenderer* renderer, const string& name)
{
	LX_CHECK_NULL_ARG(renderer);

	if(name.IsEmpty()) {
		m_AnonRenderers.PushBack(renderer);
		return ANON_BASE_ID + (m_AnonRenderers.Size() - 1);
	} else {
		size_t i = m_MaterialRenderers.Size();
		if(i >= ANON_BASE_ID)
			throw core::RuntimeException("Too many material renderers.");

		m_MaterialRenderers.PushBack(Entry(renderer, name));
		return m_MaterialRenderers.Size() - 1;
	}
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::CloneMaterialRenderer(const string& name, const string& oldName)
{
	return CloneMaterialRenderer(name, GetMaterialRenderer(oldName));
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::CloneMaterialRenderer(const string& name, const MaterialRenderer* old)
{
	LX_CHECK_NULL_ARG(old);

	StrongRef<MaterialRenderer> renderer = old->Clone();
	AddMaterialRenderer(renderer, name);
	return renderer;
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
		renderer = baseMaterial->Clone(shader, &shader->GetParamPackage());

	AddMaterialRenderer(renderer, name);

	return renderer;
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
	if(index >= ANON_BASE_ID)
		return m_AnonRenderers.At(index - ANON_BASE_ID);
	else
		return m_MaterialRenderers.At(index).renderer;
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::GetMaterialRenderer(const string& name) const
{
	auto it = core::LinearSearch(
		name,
		m_MaterialRenderers.First(), m_MaterialRenderers.End(),
		[](const Entry& other, const string& name) {
		return other.name == name;
	}
	);

	if(it == m_MaterialRenderers.End())
		throw core::ObjectNotFoundException(name.Data());

	return it->renderer;
}

size_t MaterialLibraryImpl::GetRendererID(MaterialRenderer* renderer) const
{
	for(size_t i = 0; i < m_MaterialRenderers.Size(); ++i) {
		if(renderer == m_MaterialRenderers[i].renderer)
			return i;
	}

	for(size_t i = 0; i < m_AnonRenderers.Size(); ++i) {
		if(renderer == m_AnonRenderers[i])
			return ANON_BASE_ID + i;
	}

	throw core::ObjectNotFoundException("[renderer]");
}

const string& MaterialLibraryImpl::GetRendererName(MaterialRenderer* renderer) const
{
	for(size_t i = 0; i < m_MaterialRenderers.Size(); ++i) {
		if(renderer == m_MaterialRenderers[i].renderer)
			return m_MaterialRenderers[i].name;
	}

	throw core::ObjectNotFoundException("[renderer]");
}

size_t MaterialLibraryImpl::GetMaterialRendererCount() const
{
	return m_MaterialRenderers.Size();
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::GetSolidRenderer()
{
	if(!m_Solid)
		m_Solid = GetMaterialRenderer("solid");

	return m_Solid;
}

} // !namespace video
} // !namespace lux
