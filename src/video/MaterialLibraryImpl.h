#ifndef INCLUDED_CMATERIALLIBRARY_H
#define INCLUDED_CMATERIALLIBRARY_H
#include "video/MaterialLibrary.h"
#include "core/lxArray.h"

namespace lux
{
namespace video
{
class VideoDriver;

class MaterialLibraryImpl : public MaterialLibrary
{
public:
	MaterialLibraryImpl(VideoDriver* driver);

	StrongRef<Material> CreateMaterial(const string& name);
	StrongRef<Material> CreateMaterial(MaterialRenderer* renderer = nullptr);

	StrongRef<MaterialRenderer> AddMaterialRenderer(MaterialRenderer* renderer);
	StrongRef<MaterialRenderer> CloneMaterialRenderer(const string& name, const string& oldName);
	StrongRef<MaterialRenderer> CloneMaterialRenderer(const string& name, const MaterialRenderer* pOld);
	StrongRef<MaterialRenderer> AddShaderMaterialRenderer(
		video::EShaderLanguage language,
		const io::path& VSPath, const string& VSEntryPoint, int VSMajor, int VSMinor,
		const io::path& PSPath, const string& PSEntryPoint, int PSMajor, int PSMinor,
		const MaterialRenderer* baseMaterial, const string& name,
		core::array<string>* errorList);

	StrongRef<MaterialRenderer> AddShaderMaterialRenderer(
		Shader* shader,
		const MaterialRenderer* baseMaterial, const string& name);
	void RemoveMaterialRenderer(MaterialRenderer* renderer);
	StrongRef<MaterialRenderer> GetMaterialRenderer(size_t index) const;
	StrongRef<MaterialRenderer> GetMaterialRenderer(const string& wname) const;

	size_t GetMaterialRendererCount() const;

	StrongRef<MaterialRenderer> GetSolidRenderer();

private:
	bool FindRenderer(const string& name, size_t& id) const;

private:
	core::array<StrongRef<MaterialRenderer>> m_Renderers;
	StrongRef<MaterialRenderer> m_Solid;
	StrongRef<VideoDriver> m_VideoDriver;
};

} // namespace video
} // namespace lux

#endif // INCLUDED_CMATERIALLIBRARY_H
