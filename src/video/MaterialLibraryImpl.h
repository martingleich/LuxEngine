#ifndef INCLUDED_CMATERIALLIBRARY_H
#define INCLUDED_CMATERIALLIBRARY_H
#include "video/MaterialLibrary.h"
#include "core/lxArray.h"

namespace lux
{
namespace io
{
class FileSystem;
}
namespace video
{
class VideoDriver;

class MaterialLibraryImpl : public MaterialLibrary
{
public:
	MaterialLibraryImpl(VideoDriver* driver, io::FileSystem* fileSys);
	~MaterialLibraryImpl();

	size_t AddMaterialRenderer(MaterialRenderer* renderer, const string& name);
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
	StrongRef<MaterialRenderer> GetMaterialRenderer(size_t index) const;
	StrongRef<MaterialRenderer> GetMaterialRenderer(const string& wname) const;

	size_t GetRendererID(MaterialRenderer* renderer) const;
	const string& GetRendererName(MaterialRenderer* renderer) const;

	size_t GetMaterialRendererCount() const;

private:
	struct Entry
	{
		StrongRef<MaterialRenderer> renderer;
		string name;

		Entry()
		{
		}
		Entry(MaterialRenderer* render, const string& name) :
			renderer(render),
			name(name)
		{
		}

		bool operator<(const Entry& other) const
		{
			return name < other.name;
		}

		bool operator==(const Entry& other) const
		{
			return name == other.name;
		}
	};

	core::array<Entry> m_MaterialRenderers;
	core::array<StrongRef<MaterialRenderer>> m_AnonRenderers;
	StrongRef<VideoDriver> m_VideoDriver;
	StrongRef<io::FileSystem> m_FileSystem;

	static const size_t ANON_BASE_ID = 1048576;

};

}

}


#endif // INCLUDED_CMATERIALLIBRARY_H
