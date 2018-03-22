#include "video/images/ImageSystem.h"

#include "core/ResourceSystem.h"
#include "core/ReferableFactory.h"

#include "video/VideoDriver.h"
#include "video/ColorConverter.h"

#include "io/File.h"
#include "io/FileUtilities.h"
#include "io/FileSystem.h"

#include "video/CubeTexture.h"
#include "video/SpriteBankImpl.h"

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
	core::ResourceLoader* m_ResourceLoader;
	core::Name GetFileType(io::File* file)
	{
		auto fileCursor = file->GetCursor();
		StrongRef<ResourceLoader> result;

		auto resSys = core::ResourceSystem::Instance();
		auto count = resSys->GetResourceLoaderCount();
		for(int i = 0; i < count; ++i) {
			m_ResourceLoader = resSys->GetResourceLoader(count - i - 1);
			if(m_ResourceLoader == this)
				continue;
			core::Name fileType = m_ResourceLoader->GetResourceType(file);
			if(fileType != core::Name::INVALID)
				return fileType;

			file->Seek(fileCursor, io::ESeekOrigin::Start);
		}
		m_ResourceLoader = nullptr;
		return core::Name::INVALID;
	}

	core::Name GetResourceType(io::File* file, core::Name requestedType)
	{
		if(requestedType && requestedType != core::ResourceType::Texture)
			return core::Name::INVALID;

		auto type = GetFileType(file);
		if(type == core::ResourceType::Image)
			return core::ResourceType::Texture;

		return core::Name::INVALID;
	}

	void LoadResource(io::File* file, core::Resource* dst)
	{
		auto img = core::ReferableFactory::Instance()->Create(core::ResourceType::Image, nullptr).StaticCastStrong<video::Image>();
		m_ResourceLoader->LoadResource(file, img);
		Texture* texture = dynamic_cast<Texture*>(dst);
		ColorFormat format = img->GetColorFormat();
		math::Dimension2I size = img->GetSize();

		bool result = VideoDriver::Instance()->GetFittingTextureFormat(format, size, false, false);
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

	const core::String& GetName() const
	{
		static const core::String name = "Internal texture loader";
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
	static const char* GetName(int i)
	{
		static const char* NAMES[] = {"right", "left", "top", "bottom", "front", "back"};
		return NAMES[i];
	}

	static const char* GetAxis(int i)
	{
		static const char* AXES[] = {"posx", "negx", "posy", "negy", "posz", "negz"};
		return AXES[i];
	}

	ENameScheme GetNameScheme(const io::Path& path, core::String& outBaseName, int& outId)
	{
		io::Path basePath = io::GetFileDir(path);
		core::String nameonly = io::GetFilenameOnly(path, false);
		core::String ext = io::GetFileExtension(path);
		if(*nameonly.Last() > '1' && *nameonly.Last() <= '6') {
			outBaseName = nameonly.SubString(nameonly.First(), nameonly.Last());
			outId = *nameonly.Last() - '1';
			return ENameScheme::Numeric;
		}
		for(int i = 0; i < 6; ++i) {
			if(nameonly.EndsWith(GetName(i))) {
				outBaseName = nameonly.SubString(nameonly.First(), nameonly.End() - strlen(GetName(i)));
				outId = i;
				return ENameScheme::Names;
			}
		}

		for(int i = 0; i < 6; ++i) {
			if(nameonly.EndsWith(GetAxis(i))) {
				outBaseName = nameonly.SubString(nameonly.First(), nameonly.End() - strlen(GetAxis(i)));
				outId = i;
				return ENameScheme::Axes;
			}
		}

		return ENameScheme::Invalid;
	}

	core::Name GetResourceType(io::File* file, core::Name requestedType)
	{
		if(requestedType && requestedType != core::ResourceType::CubeTexture)
			return core::Name::INVALID;

		auto lineEnding = io::GetLineEnding(file);
		if(lineEnding == io::ELineEnding::Unknown)
			return core::Name::INVALID;
		core::Array<core::String> lines;
		for(auto& l : Lines(file, lineEnding)) {
			if(!l.IsWhitespace())
				lines.PushBack(l);
		}
		if(lines.Size() != 6)
			return core::Name::INVALID;
		return core::ResourceType::CubeTexture;
	}

	void LoadResource(io::File* file, core::Resource* dst)
	{
		int order[6] = {0, 1, 2, 3, 4, 5};
		core::String lines[6];
		StrongRef<Image> images[6];

		auto lineEnding = io::GetLineEnding(file);
		int i = 0;
		for(auto& l : Lines(file, lineEnding)) {
			if(!l.IsWhitespace()) {
				if(i == 6)
					continue;
				lines[i++] = l;
			}
		}

		core::String baseName;
		int id;
		ENameScheme scheme = GetNameScheme(lines[0], baseName, id);
		if(scheme != ENameScheme::Invalid) {
			core::String baseName2;
			i = 0;
			for(auto& l : lines) {
				ENameScheme scheme2 = GetNameScheme(l, baseName2, order[i++]);
				if(scheme2 != scheme || baseName != baseName2) {
					scheme = ENameScheme::Invalid;
					break;
				}
			}
		}

		auto baseDir = file->GetDescription();
		auto fileSys = io::FileSystem::Instance();
		for(i = 0; i < 6; ++i) {
			auto& path = lines[i];
			auto imgfile = fileSys->OpenFile(io::ConcatFileDesc(baseDir, path));
			if(imgfile)
				images[order[i]] = core::ResourceSystem::Instance()->GetResource(core::ResourceType::Image, imgfile).AsStrong<Image>();
		}

		InitCubeTexture(images, (CubeTexture*)dst);
	}

	const core::String& GetName() const
	{
		static const core::String name = "Internal list to cube texture loader";
		return name;
	}

private:
	void InitCubeTexture(StrongRef<Image> images[6], CubeTexture* tex)
	{
		ColorFormat format = ColorFormat::A8R8G8B8;
		u32 size = 0;
		int real_count = 0;
		for(int i = 0; i < 6; ++i) {
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

		math::Dimension2I ds(size, size);
		if(!VideoDriver::Instance()->GetFittingTextureFormat(format, ds, true, false))
			throw core::ColorFormatException(format);

		tex->Init(size, format, false, false);
		for(int i = 0; i < 6; ++i) {
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
} // anonynmous namespace

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
	m_Driver = VideoDriver::Instance();

	// Register before image loaders, to make default load type images, instead of textures.
	// These both are always the last option to try and load.
	resSys->AddResourceLoader(LUX_NEW(ImageToTextureLoader));
	resSys->AddResourceLoader(LUX_NEW(MultiImageToCubeTextureLoader));

#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER
	auto driver = VideoDriver::Instance();
	if(driver && driver->GetVideoDriverType() == video::DriverType::Direct3D9) {
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

StrongRef<Image> ImageSystem::CreateImage(const math::Dimension2I& size, ColorFormat format)
{
	auto img = core::ReferableFactory::Instance()->Create(core::ResourceType::Image).AsStrong<Image>();
	img->Init(size, format);
	return img;
}

StrongRef<Image> ImageSystem::CreateImage(const math::Dimension2I& size, ColorFormat format, void* data, bool CopyMem, bool deleteOnDrop)
{
	auto img = core::ReferableFactory::Instance()->Create(core::ResourceType::Image).AsStrong<Image>();
	img->Init(size, format, data, CopyMem, deleteOnDrop);
	return img;
}

StrongRef<SpriteBank> ImageSystem::CreateSpriteBank()
{
	return LUX_NEW(SpriteBankImpl);
}

StrongRef<Texture> ImageSystem::CreateFittingTexture(const math::Dimension2I& size, ColorFormat format, u32 mipCount, bool isDynamic)
{
	math::Dimension2I copy(size);
	if(!m_Driver->GetFittingTextureFormat(format, copy, false, false))
		throw core::RuntimeException("No matching texture format found");

	return m_Driver->CreateTexture(copy, format, mipCount, isDynamic);
}

StrongRef<CubeTexture> ImageSystem::CreateFittingCubeTexture(u32 size, ColorFormat format, bool isDynamic)
{
	math::Dimension2I copy(size, size);
	if(!m_Driver->GetFittingTextureFormat(format, copy, true, false))
		throw core::RuntimeException("No matching texture format found");

	return m_Driver->CreateCubeTexture(copy.width, format, isDynamic);
}

StrongRef<Texture> ImageSystem::CreateFittingRendertargetTexture(const math::Dimension2I& size, ColorFormat format)
{
	math::Dimension2I copy(size);
	if(!m_Driver->GetFittingTextureFormat(format, copy, false, true))
		throw core::RuntimeException("No matching texture format found");

	return m_Driver->CreateRendertargetTexture(copy, format);
}

} // namespace video
} // namespace lux
