#ifndef INCLUDED_CIMAGESYSTEM_H
#define INCLUDED_CIMAGESYSTEM_H
#include "video/images/ImageSystem.h"
#include "core/lxArray.h"

namespace lux
{
namespace video
{

class ImageSystemImpl : public ImageSystem
{
	friend class ImageToTextureLoader;
	friend class MultiImageToCubeTextureLoader;
public:
	ImageSystemImpl(io::FileSystem* fileSystem, video::VideoDriver* driver, core::ResourceSystem* resSys);
	~ImageSystemImpl();
	StrongRef<Image> CreateImage(const math::dimension2du& size, ColorFormat format);
	StrongRef<Image> CreateImage(const math::dimension2du& size, ColorFormat format, void* data, bool CopyMem = true, bool deleteOnDrop = true);

	bool WriteImageToFile(Image* image, io::File* file);
	bool WriteImageToFile(Image* image, const io::path& file);
	bool WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, io::File* file);
	bool WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, const io::path& file);

	void AddExternalImageWriter(ImageWriter* writer);
	size_t GetImageWriterCount() const;
	ImageWriter* GetImageWriter(size_t index);
	ImageWriter* GetImageWriter(const io::path& name);

	Texture* AddTexture(const string& name, const math::dimension2du& size, ColorFormat format, bool isDynamic = false);
	Texture* AddTexture(const string& name, Image* image, bool isDynamic = false);

	StrongRef<video::Texture> AddChromaKeyedTexture(video::Image* image, video::Color key);
	StrongRef<video::Texture> GetChromaKeyedTexture(const io::path& p, video::Color key);
	StrongRef<video::Texture> GetChromaKeyedTexture(const io::path& p, const math::vector2i& pos);

	CubeTexture* AddCubeTexture(const string& name, StrongRef<Image> images[6]);
	CubeTexture* AddCubeTexture(const string& name, ColorFormat format, u32 size);

	Texture* AddRendertargetTexture(const string& name, const math::dimension2du& size, ColorFormat format);

	StrongRef<SpriteBank> CreateSpriteBank();

	bool SetTextureCreationFlag(ETextureCreationFlag flag, bool set);
	bool GetTextureCreationFlag(ETextureCreationFlag flag) const;

private:
	StrongRef<Texture> CreateTexture(ColorFormat format, math::dimension2du size, bool isDynamic, void* pImageBits);
	StrongRef<Texture> CreateTexture(Image* image, bool isDynamic);
	StrongRef<CubeTexture> CreateCubeTexture(ColorFormat format, u32 size);
	StrongRef<CubeTexture> CreateCubeTexture(StrongRef<Image> images[6]);

	bool InitCubeTexture(StrongRef<Image> images[6], CubeTexture* tex);
	bool GetFittingTextureFormat(ColorFormat& format, math::dimension2du& size, bool cube);

private:
	core::array<StrongRef<ImageWriter>> m_WriterList;

	StrongRef<core::ResourceSystem> m_ResourceSystem;
	StrongRef<io::FileSystem> m_Filesystem;
	WeakRef<video::VideoDriver> m_Driver; // VideoDriver owns ImageSystem prevent circular reference.

	u32 m_TextureCreationFlags;

	core::array<StrongRef<Texture>> m_Rendertargets;
};

}
}

#endif // !INCLUDED_CIMAGESYSTEM_H

