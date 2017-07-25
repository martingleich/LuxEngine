#include "video/images/ImageSystem.h"

#include "video/VideoDriver.h"
#include "video/ColorConverter.h"

#include "io/File.h"
#include "io/FileSystem.h"
#include "resources/ResourceSystem.h"

#include "video/CubeTexture.h"

#include "video/SpriteBankImpl.h"

#include "video/images/ImageImpl.h"

#include "video/images/ImageLoaderBMP.h"
#include "video/images/ImageLoaderPNM.h"
#include "video/images/ImageLoaderTGA.h"
#include "video/images/ImageLoaderPNG.h"

#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER
#include "video/images/ImageLoaderD3DX.h"
#endif

#include "video/images/ImageWriterBMP.h"
#include "video/images/ImageWriterTGA.h"

namespace lux
{
namespace video
{

namespace
{
class ImageToTextureLoader : public core::ResourceLoader
{
public:
	core::Name GetFileType(io::File* file)
	{
		const u32 fileCursor = file->GetCursor();
		StrongRef<ResourceLoader> result;

		auto resSys = core::ResourceSystem::Instance();
		u32 count = resSys->GetResourceLoaderCount();
		for(u32 i = 0; i < count; ++i) {
			auto loader = resSys->GetResourceLoader(count - i - 1);
			if(loader == this)
				continue;
			core::Name fileType = loader->GetResourceType(file);
			if(fileType != core::Name::INVALID)
				return fileType;

			if(!file->Seek(fileCursor, io::ESeekOrigin::Start))
				break;
		}

		return core::Name::INVALID;
	}

	core::Name GetResourceType(io::File* file, core::Name requestedType)
	{
		if(requestedType && requestedType != core::ResourceType::Texture && requestedType != core::ResourceType::CubeTexture)
			return core::Name::INVALID;

		core::Name type = GetFileType(file);
		if(type == core::ResourceType::Image)
			return core::ResourceType::Texture;
		if(type == core::ResourceType::ImageList)
			return core::ResourceType::CubeTexture;

		return core::Name::INVALID;
	}

	void LoadResource(io::File* file, core::Resource* dst)
	{
		StrongRef<core::Resource> r = core::ResourceSystem::Instance()->GetResource(core::ResourceType::Image, file);
		StrongRef<Image> img = r;
		Texture* texture = dynamic_cast<Texture*>(dst);
		ColorFormat format = img->GetColorFormat();
		math::dimension2du size = img->GetSize();

		bool result = VideoDriver::Instance()->GetFittingTextureFormat(format, size, false);
		if(!result)
			throw core::FileFormatException("No matching texture format", "texture_loader_proxy");
		texture->Init(size, format, 0, false, false);

		ImageLock lock(img);
		TextureLock texLock(texture, BaseTexture::ELockMode::Overwrite);

		ColorConverter::ConvertByFormat(
			lock.data, img->GetColorFormat(),
			texLock.data, texture->GetColorFormat(),
			img->GetSize().width,
			img->GetSize().height,
			0, texLock.pitch);
	}

	const String& GetName() const
	{
		static const String name = "Internal texture loader";
		return name;
	}
};

class MultiImageToCubeTextureLoader : public core::ResourceLoader
{
private:
	enum class ENameScheme
	{
		Invalid, // No valid name scheme
		Numeric, // file1, file2, file3
		Names, // file_front, file_back, file_top, file_bottom, file_left, file_right
		Axes, // file_posx, file_negx, file_negy
	};

public:
	static const char* GetName(size_t i)
	{
		static const char* NAMES[] = {"right", "left", "top", "bottom", "front", "back"};
		return NAMES[i];
	}

	static const char* GetAxis(size_t i)
	{
		static const char* AXES[] = {"posx", "negx", "posy", "negy", "posz", "negz"};
		return AXES[i];
	}

	String GetName(ENameScheme scheme, size_t id, const io::Path& basePath, const String& name, const String& ext)
	{
		switch(scheme) {
		case ENameScheme::Numeric:
			return basePath + name + core::StringConverter::ToString(id + 1) + "." + ext;
		case ENameScheme::Names:
			return basePath + name + GetName(id) + "." + ext;
		case ENameScheme::Axes:
			return basePath + name + GetAxis(id) + "." + ext;
		default:
			lxAssertNeverReach("Unsupported cube texture naming scheme.");
			return "";
		}
	}

	ENameScheme GetNameSchema(const io::Path& path, String& outBaseName, size_t& outId)
	{
		io::Path basePath = io::GetFileDir(path);
		String nameonly = io::GetFilenameOnly(path, false);
		String ext = io::GetFileExtension(path);
		if(*nameonly.Last() > '1' && *nameonly.Last() <= '6') {
			outBaseName = nameonly.SubString(nameonly.First(), nameonly.Last());
			outId = *nameonly.Last() - '1';
			return ENameScheme::Numeric;
		}
		for(size_t i = 0; i < 6; ++i)
			if(nameonly.EndsWith(GetName(i))) {
				outBaseName = nameonly.SubString(nameonly.First(), nameonly.End() - strlen(GetName(i)));
				outId = i;
				return ENameScheme::Names;
			}

		for(size_t i = 0; i < 6; ++i)
			if(nameonly.EndsWith(GetAxis(i))) {
				outBaseName = nameonly.SubString(nameonly.First(), nameonly.End() - strlen(GetAxis(i)));
				outId = i;
				return ENameScheme::Axes;
			}

		return ENameScheme::Invalid;
	}

	core::Name GetResourceType(io::File* file, core::Name requestedType)
	{
		if(requestedType && requestedType != core::ResourceType::CubeTexture)
			return core::Name::INVALID;

		auto fileSys = io::FileSystem::Instance();
		auto filename = file->GetName();
		if(fileSys->ExistFile(filename)) {
			String baseName;
			size_t id;
			ENameScheme scheme = GetNameSchema(filename, baseName, id);
			if(scheme == ENameScheme::Invalid)
				return core::Name::INVALID;

			String otherName = GetName(scheme, (id + 1) % 6, io::GetFileDir(filename), baseName, io::GetFileExtension(filename));
			if(!fileSys->ExistFile(otherName))
				return core::Name::INVALID;

			return core::ResourceType::CubeTexture;
		}

		return core::Name::INVALID;
	}

	void LoadResource(io::File* file, core::Resource* dst)
	{
		auto fileSys = io::FileSystem::Instance();
		auto filename = file->GetName();

		io::Path basePath = io::GetFileDir(filename);
		String ext = io::GetFileExtension(filename);
		String baseName;
		size_t id;
		ENameScheme scheme = GetNameSchema(filename, baseName, id);

		bool isValid = true;
		io::Path image_path[6];
		StrongRef<Image> images[6];
		for(size_t i = 0; i < 6; ++i) {
			image_path[i] = GetName(scheme, i, basePath, baseName, ext);
			if(!fileSys->ExistFile(image_path[i]))
				isValid = false;
		}

		if(isValid) {
			for(size_t i = 0; i < 6; ++i)
				images[i] = core::ResourceSystem::Instance()->GetResource(core::ResourceType::Image, image_path[i]);
		} else {
			throw core::FileNotFoundException(filename.Data());
		}

		InitCubeTexture(images, (CubeTexture*)dst);
	}

	const String& GetName() const
	{
		static const String name = "Internal list to cube texture loader";
		return name;
	}

private:
	void InitCubeTexture(StrongRef<Image> images[6], CubeTexture* tex)
	{
		ColorFormat format = ColorFormat::A8R8G8B8;
		u32 size = 0;
		u32 real_count = 0;
		for(u32 i = 0; i < 6; ++i) {
			if(images[i]) {
				if(real_count == 0) {
					format = images[i]->GetColorFormat();
					size = images[i]->GetSize().GetAvgEdge();
				}

				++real_count;
			}
		}
		if(real_count != 6)
			throw core::InvalidArgumentException("images", "Must not be null");

		math::dimension2du ds(size, size);
		if(!VideoDriver::Instance()->GetFittingTextureFormat(format, ds, true))
			throw core::ColorFormatException(format);

		tex->Init(size, format, false);
		for(u32 i = 0; i < 6; ++i) {
			CubeTextureLock lock(tex, BaseTexture::ELockMode::Overwrite, (CubeTexture::EFace)i);
			ImageLock imgLock(images[i]);

			ColorConverter::ConvertByFormat(
				imgLock.data, images[i]->GetColorFormat(),
				lock.data, tex->GetColorFormat(),
				images[i]->GetSize().width, images[i]->GetSize().height,
				0, lock.pitch);
		}
	}
};
}

static StrongRef<ImageSystem> g_ImageSystem;

void ImageSystem::Initialize(ImageSystem* system)
{
	if(!system)
		system = LUX_NEW(ImageSystem);
	g_ImageSystem = system;
}

ImageSystem* ImageSystem::Instance()
{
	return g_ImageSystem;
}
void ImageSystem::Destroy()
{
	g_ImageSystem.Reset();
}

ImageSystem::ImageSystem()
{
	auto resSys = core::ResourceSystem::Instance();

	// Register before image loaders, to make default load type images, instead of textures.
	resSys->AddResourceLoader(LUX_NEW(ImageToTextureLoader));
	resSys->AddResourceLoader(LUX_NEW(MultiImageToCubeTextureLoader));

#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER
	auto driver = VideoDriver::Instance();
	if(driver && driver->GetVideoDriverType() == EDriverType::Direct3D9) {
		IDirect3DDevice9* d3dDevice = reinterpret_cast<IDirect3DDevice9*>(driver->GetLowLevelDevice());
		resSys->AddResourceLoader(LUX_NEW(ImageLoaderD3DX)(d3dDevice));
	}
#endif // LUX_COMPILE_WITH_D3DX_IMAGE_LOADER

	resSys->AddResourceLoader(LUX_NEW(ImageLoaderBMP));
	resSys->AddResourceLoader(LUX_NEW(ImageLoaderPNM));
	resSys->AddResourceLoader(LUX_NEW(ImageLoaderTGA));
	resSys->AddResourceLoader(LUX_NEW(ImageLoaderPNG));

	resSys->AddResourceWriter(LUX_NEW(ImageWriterBMP));
	resSys->AddResourceWriter(LUX_NEW(ImageWriterTGA));
}

ImageSystem::~ImageSystem()
{
}

StrongRef<Image> ImageSystem::CreateImage(const math::dimension2du& size, ColorFormat format)
{
	StrongRef<Image> img = LUX_NEW(ImageImpl);
	img->Init(size, format);
	return img;
}

StrongRef<Image> ImageSystem::CreateImage(const math::dimension2du& size, ColorFormat format, void* data, bool CopyMem, bool deleteOnDrop)
{
	StrongRef<Image> img = LUX_NEW(ImageImpl);
	img->Init(size, format, data, CopyMem, deleteOnDrop);
	return img;
}

StrongRef<SpriteBank> ImageSystem::CreateSpriteBank()
{
	return LUX_NEW(SpriteBankImpl);
}

} // namespace video
} // namespace lux
