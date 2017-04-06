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

	int AddMaterialRenderer(MaterialRenderer* renderer, const string& name);
	StrongRef<MaterialRenderer> CloneMaterialRenderer(const string& name, const string& oldName);
	StrongRef<MaterialRenderer> CloneMaterialRenderer(const string& name, const MaterialRenderer* pOld);
	StrongRef<MaterialRenderer> AddShaderMaterialRenderer(const string& VSPath, const string& VSEntryPoint, video::EVertexShaderType VSType,
		const string& PSPath, const string& PSEntryPoint, video::EPixelShaderType PSType,
		const MaterialRenderer* baseMaterial, const string& name);
	StrongRef<MaterialRenderer> GetMaterialRenderer(u32 index) const;
	StrongRef<MaterialRenderer> GetMaterialRenderer(const string& wname) const;

	int GetRendererID(MaterialRenderer* renderer) const;
	const string& GetRendererName(MaterialRenderer* renderer) const;

	u32 GetMaterialRendererCount() const;

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

	static const int ANON_BASE_ID = 1048576;

};

} // namespace video
} // namespace lux

#endif // INCLUDED_CMATERIALLIBRARY_H
