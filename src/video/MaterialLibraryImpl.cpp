#include "MaterialLibraryImpl.h"
#include "core/lxAlgorithm.h"
#include "video/VideoDriver.h"
#include "io/filesystem.h"
#include "io/file.h"
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

MaterialLibraryImpl::~MaterialLibraryImpl()
{
}

int MaterialLibraryImpl::AddMaterialRenderer(MaterialRenderer* renderer, const string& name)
{
	if(!renderer)
		return -1;

	if(name.IsEmpty()) {
		m_AnonRenderers.Push_Back(renderer);
		return ANON_BASE_ID + (m_AnonRenderers.Size() - 1);
	} else {
		int i = m_MaterialRenderers.Size();
		if(i >= ANON_BASE_ID)
			return -1;

		m_MaterialRenderers.Push_Back(Entry(renderer, name));
		return m_MaterialRenderers.Size() - 1;
	}
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::CloneMaterialRenderer(const string& name, const string& oldName)
{
	return CloneMaterialRenderer(name, GetMaterialRenderer(oldName));
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::CloneMaterialRenderer(const string& name, const MaterialRenderer* old)
{
	if(!old)
		return nullptr;

	StrongRef<MaterialRenderer> renderer = old->Clone();

	if(AddMaterialRenderer(renderer, name) > 0)
		return renderer;
	else
		return nullptr;
}


StrongRef<MaterialRenderer> MaterialLibraryImpl::AddShaderMaterialRenderer(const io::path& VSPath, const string& VSEntryPoint, video::EVertexShaderType VSType,
	const io::path& PSPath, const string& PSEntryPoint, video::EPixelShaderType PSType,
	const MaterialRenderer* baseMaterial, const string& name)
{
	if(!baseMaterial)
		return nullptr;

	if(baseMaterial->GetShader() != nullptr)
		return nullptr;

	StrongRef<io::File> PSFile;
	StrongRef<io::File> VSFile;
	if(VSPath == PSPath) {
		PSFile = VSFile = (io::File*)m_FileSystem->OpenFile(VSPath);
		if(!PSFile) {
			log::Error("Can't open file: ~s.", VSPath);
			return nullptr;
		}
	} else {
		PSFile = (io::File*)m_FileSystem->OpenFile(PSPath);
		if(!PSFile) {
			log::Error("Can't open file: ~s.", PSPath);
			return nullptr;
		}

		VSFile = (io::File*)m_FileSystem->OpenFile(VSPath);
		if(!VSFile) {
			log::Error("Can't open file: ~s.", PSPath);
			return nullptr;
		}
	}


	void* vsCode = VSFile->GetBuffer();
	if(!vsCode) {
		vsCode = LUX_NEW_ARRAY(u8, VSFile->GetSize() + 1);
		VSFile->ReadBinary(VSFile->GetSize(), vsCode);
		((char*)vsCode)[VSFile->GetSize()] = 0;
	}

	void* psCode = PSFile->GetBuffer();
	if(!psCode) {
		if(PSFile == VSFile) {
			psCode = vsCode;
		} else {
			psCode = LUX_NEW_ARRAY(u8, PSFile->GetSize()+1);
			PSFile->ReadBinary(PSFile->GetSize(), psCode);
			((char*)psCode)[PSFile->GetSize()] = 0;
		}
	}
	StrongRef<video::Shader> shader = m_VideoDriver->CreateShader((const char*)vsCode, VSEntryPoint.Data(), VSFile->GetSize(), VSType,
		(const char*)psCode, PSEntryPoint.Data(), PSFile->GetSize(), PSType);

	if(!VSFile->GetBuffer())
		LUX_FREE_ARRAY(vsCode);
	if(!PSFile->GetBuffer()) {
		if(VSFile != PSFile)
			LUX_FREE_ARRAY(psCode);
	}

	StrongRef<video::MaterialRenderer> renderer;
	if(shader)
		renderer = baseMaterial->Clone(shader, &shader->GetParamPackage());

	if(AddMaterialRenderer(renderer, name) > 0)
		return renderer;
	else
		return nullptr;
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::GetMaterialRenderer(u32 index) const
{
	if(index >= ANON_BASE_ID) {
		index -= ANON_BASE_ID;
		if(index < m_AnonRenderers.Size())
			return m_AnonRenderers[index];
	} else {
		if(index < m_MaterialRenderers.Size())
			return m_MaterialRenderers[index].renderer;
	}

	return nullptr;
}

StrongRef<MaterialRenderer> MaterialLibraryImpl::GetMaterialRenderer(const string& name) const
{
	auto it = core::Linear_Search(
		name,
		m_MaterialRenderers.First(), m_MaterialRenderers.End(),
		[](const Entry& other, const string& name) {
		return other.name == name;
	}
	);

	if(it != m_MaterialRenderers.End())
		return it->renderer;
	else
		return nullptr;
}

int MaterialLibraryImpl::GetRendererID(MaterialRenderer* renderer) const
{
	for(u32 i = 0; i < m_MaterialRenderers.Size(); ++i) {
		if(renderer == m_MaterialRenderers[i].renderer)
			return i;
	}

	for(u32 i = 0; i < m_AnonRenderers.Size(); ++i) {
		if(renderer == m_AnonRenderers[i])
			return ANON_BASE_ID + i;
	}

	return -1;
}

const string& MaterialLibraryImpl::GetRendererName(MaterialRenderer* renderer) const
{
	for(u32 i = 0; i < m_MaterialRenderers.Size(); ++i) {
		if(renderer == m_MaterialRenderers[i].renderer)
			return m_MaterialRenderers[i].name;
	}

	return string::EMPTY;
}

u32 MaterialLibraryImpl::GetMaterialRendererCount() const
{
	return m_MaterialRenderers.Size();
}

} // !namespace video
} // !namespace lux
