#include "ImageSystemImpl.h"

#include "io/FileSystem.h"
#include "io/File.h"

#include "video/VideoDriver.h"
#include "video/ColorConverter.h"

#include "video/Texture.h"
#include "video/CubeTexture.h"

#include "video/SpriteBankImpl.h"

#include "core/Logger.h"

#include "core/lxAlgorithm.h"

#include "ImageImpl.h"

#include "ImageLoaderBMP.h"
#include "ImageLoaderPNM.h"
#include "ImageLoaderTGA.h"
#include "ImageLoaderPNG.h"

#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER
#include "ImageLoaderD3DX.h"
#endif

#include "ImageWriterBMP.h"
#include "ImageWriterTGA.h"

namespace lux
{
namespace video
{

class ImageToTextureLoader : public core::ResourceLoader
{
public:
	ImageToTextureLoader(core::ResourceSystem* resSys, ImageSystemImpl* imagSys) :
		m_ResourceSystem(resSys),
		m_ImageSystem(imagSys)
	{
	}

	core::Name GetFileType(io::File* file)
	{
		const u32 fileCursor = file->GetCursor();
		StrongRef<ResourceLoader> result;

		u32 count = m_ResourceSystem->GetResourceLoaderCount();
		for(u32 i = 0; i < count; ++i) {
			auto loader = m_ResourceSystem->GetResourceLoader(count - i - 1);
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
		StrongRef<core::Resource> r = m_ResourceSystem->GetResource(core::Name::INVALID, file);
		bool result;
		if(r) {
			if(r->GetReferableSubType() == core::ResourceType::Image) {

				StrongRef<Image> img = r;
				Texture* texture = dynamic_cast<Texture*>(dst);
				ColorFormat format = img->GetColorFormat();
				math::dimension2du size = img->GetSize();

				result = m_ImageSystem->GetFittingTextureFormat(format, size, false);
				if(!result)
					throw core::LoaderException();
				texture->Init(size, format, 0, false, false);

				video::ImageLock lock(img);
				video::TextureLock texLock(texture, BaseTexture::ETLM_OVERWRITE);

				ColorConverter::ConvertByFormat(
					lock.data, img->GetColorFormat(),
					texLock.data, texture->GetColorFormat(),
					img->GetSize().width,
					img->GetSize().height,
					0, texLock.pitch);

				return;
			}
		}

		throw core::LoaderException();
	}

	const string& GetName() const
	{
		static const string name = "Internal texture loader";
		return name;
	}

private:
	ImageSystemImpl* m_ImageSystem;
	core::ResourceSystem* m_ResourceSystem;
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
	MultiImageToCubeTextureLoader(core::ResourceSystem* resSys, ImageSystemImpl* imagSys) :
		m_ImageSystem(imagSys),
		m_ResSys(resSys)
	{
	}

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

	string GetName(ENameScheme scheme, size_t id, const io::path& basePath, const string& name, const string& ext)
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

	ENameScheme GetNameSchema(const io::path& path, string& outBaseName, size_t& outId)
	{
		io::path basePath = io::GetFileDir(path);
		string nameonly = io::GetFilenameOnly(path, false);
		string ext = io::GetFileExtension(path);
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

		auto fileSys = m_ImageSystem->m_Filesystem;
		auto filename = file->GetName();
		if(fileSys->ExistFile(filename)) {
			string baseName;
			size_t id;
			ENameScheme scheme = GetNameSchema(filename, baseName, id);
			if(scheme == ENameScheme::Invalid)
				return core::Name::INVALID;

			string otherName = GetName(scheme, (id + 1) % 6, io::GetFileDir(filename), baseName, io::GetFileExtension(filename));
			if(!fileSys->ExistFile(otherName))
				return core::Name::INVALID;

			return core::ResourceType::CubeTexture;
		}

		return core::Name::INVALID;
	}

	void LoadResource(io::File* file, core::Resource* dst)
	{
		auto fileSys = m_ImageSystem->m_Filesystem;
		auto filename = file->GetName();

		io::path basePath = io::GetFileDir(filename);
		string ext = io::GetFileExtension(filename);
		string baseName;
		size_t id;
		ENameScheme scheme = GetNameSchema(filename, baseName, id);

		bool isValid = true;
		io::path image_path[6];
		StrongRef<Image> images[6];
		for(size_t i = 0; i < 6; ++i) {
			image_path[i] = GetName(scheme, i, basePath, baseName, ext);
			if(!fileSys->ExistFile(image_path[i]))
				isValid = false;
		}

		if(isValid) {
			for(size_t i = 0; i < 6; ++i)
				images[i] = m_ResSys->GetResource(core::ResourceType::Image, image_path[i]);
		} else {
			throw core::FileNotFoundException(filename.Data());
		}

		m_ImageSystem->InitCubeTexture(images, (CubeTexture*)dst);
	}

	const string& GetName() const
	{
		static const string name = "Internal list to cube texture loader";
		return name;
	}

private:
	ImageSystemImpl* m_ImageSystem;
	core::ResourceSystem* m_ResSys;
};

ImageSystemImpl::ImageSystemImpl(io::FileSystem* fileSystem, video::VideoDriver* driver, core::ResourceSystem* resSys) :
	m_Filesystem(fileSystem),
	m_Driver(driver),
	m_ResourceSystem(resSys),
	m_TextureCreationFlags(ETCF_ALPHA_CHANNEL | ETCF_CREATE_MIP_MAPS)
{
	// Register before image loaders, to make default load type images, instead of textures.
	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageToTextureLoader)(m_ResourceSystem, this));
	m_ResourceSystem->AddResourceLoader(LUX_NEW(MultiImageToCubeTextureLoader)(m_ResourceSystem, this));

#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER
	if(m_Driver && m_Driver->GetVideoDriverType() == EVideoDriver::Direct3D9) {
		IDirect3DDevice9* d3dDevice = reinterpret_cast<IDirect3DDevice9*>(m_Driver->GetDevice());
		m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderD3DX)(d3dDevice));
	}
#endif // LUX_COMPILE_WITH_D3DX_IMAGE_LOADER

	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderBMP));
	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderPNM));
	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderTGA));
	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderPNG));

	AddExternalImageWriter(LUX_NEW(ImageWriterBMP));
	AddExternalImageWriter(LUX_NEW(ImageWriterTGA));
}

StrongRef<Texture> ImageSystemImpl::CreateTexture(ColorFormat format, math::dimension2du size, bool isDynamic, void* imageBits)
{
	if(!m_Driver)
		throw core::Exception("No driver available");

	ColorFormat srcFormat = format;
	if(imageBits) {
		if(GetFittingTextureFormat(format, size, false) == false)
			throw core::ColorFormatException(format);

		if(ColorConverter::IsConvertable(srcFormat, format) == false)
			throw core::ColorFormatException(format);
	}

	StrongRef<Texture> texture = m_Driver->CreateTexture(size, format,
		GetTextureCreationFlag(ETCF_CREATE_MIP_MAPS) ? 0 : 1, isDynamic);

	if(imageBits) {
		video::TextureLock lock(texture, BaseTexture::ETLM_OVERWRITE);
		ColorConverter::ConvertByFormat(imageBits, srcFormat,
			lock.data, format,
			size.width, size.height, 0, lock.pitch);
	}

	return texture;
}

StrongRef<Texture> ImageSystemImpl::CreateTexture(Image* image, bool isDynamic)
{
	video::ImageLock lock(image);
	return CreateTexture(image->GetColorFormat(), image->GetSize(), isDynamic, lock.data);
}

StrongRef<CubeTexture> ImageSystemImpl::CreateCubeTexture(ColorFormat format, u32 size)
{
	if(!m_Driver)
		throw core::Exception("No driver available");

	math::dimension2du tmp = math::dimension2du(size, size);
	if(GetFittingTextureFormat(format, tmp, true) == false)
		throw core::ColorFormatException(format);

	return m_Driver->CreateCubeTexture(tmp.width, format, false);
}

StrongRef<CubeTexture> ImageSystemImpl::CreateCubeTexture(StrongRef<Image> images[6])
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

	StrongRef<CubeTexture> texture = CreateCubeTexture(format, size);

	for(u32 i = 0; i < 6; ++i) {
		video::CubeTextureLock lock(texture, BaseTexture::ETLM_OVERWRITE, (CubeTexture::EFace)i);
		video::ImageLock imgLock(images[i]);

		ColorConverter::ConvertByFormat(
			imgLock.data, images[i]->GetColorFormat(),
			lock.data, texture->GetColorFormat(),
			images[i]->GetSize().width, images[i]->GetSize().height,
			0, lock.pitch);
	}

	return texture;
}


void ImageSystemImpl::InitCubeTexture(StrongRef<Image> images[6], CubeTexture* tex)
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
	if(!GetFittingTextureFormat(format, ds, true))
		throw core::ColorFormatException(format);

	tex->Init(size, format, false);
	for(u32 i = 0; i < 6; ++i) {
		video::CubeTextureLock lock(tex, BaseTexture::ETLM_OVERWRITE, (CubeTexture::EFace)i);
		video::ImageLock imgLock(images[i]);

		ColorConverter::ConvertByFormat(
			imgLock.data, images[i]->GetColorFormat(),
			lock.data, tex->GetColorFormat(),
			images[i]->GetSize().width, images[i]->GetSize().height,
			0, lock.pitch);
	}
}

bool ImageSystemImpl::GetFittingTextureFormat(ColorFormat& format, math::dimension2du& size, bool cube)
{
	LUX_UNUSED(size);

	// TODO: Handle size
	// TODO: How to use floating-point formats
	if(GetTextureCreationFlag(ETCF_ALPHA_CHANNEL)) {
		if(GetTextureCreationFlag(ETCF_ALWAYS_32_BIT))
			format = ColorFormat::A8R8G8B8;
		else if(GetTextureCreationFlag(ETCF_ALWAYS_16_BIT))
			format = ColorFormat::A1R5G5B5;
		else {
			//bool alpha = GetTextureCreationFlag(ETCF_ALPHA_CHANNEL);
			if(m_Driver->CheckTextureFormat(format, cube))
				format = format;
			else if(m_Driver->CheckTextureFormat(ColorFormat::A8R8G8B8, cube))
				format = ColorFormat::A8R8G8B8;
			else if(m_Driver->CheckTextureFormat(ColorFormat::A1R5G5B5, cube))
				format = ColorFormat::A1R5G5B5;
			else
				return false;
		}
	} else {
		if(GetTextureCreationFlag(ETCF_ALWAYS_32_BIT))
			format = ColorFormat::A8R8G8B8;
		else if(GetTextureCreationFlag(ETCF_ALWAYS_16_BIT))
			format = ColorFormat::R5G6B5;
		else {
			//bool alpha = GetTextureCreationFlag(ETCF_ALPHA_CHANNEL);
			if(m_Driver->CheckTextureFormat(format, cube))
				format = format;
			else if(m_Driver->CheckTextureFormat(ColorFormat::A8R8G8B8, cube))
				format = ColorFormat::A8R8G8B8;
			else if(m_Driver->CheckTextureFormat(ColorFormat::R5G6B5, cube))
				format = ColorFormat::A1R5G5B5;
			else
				return false;
		}
	}

	return true;
}

StrongRef<Image> ImageSystemImpl::CreateImage(const math::dimension2du& size, ColorFormat format)
{
	StrongRef<Image> img = LUX_NEW(ImageImpl);
	img->Init(size, format);
	return img;
}

StrongRef<Image> ImageSystemImpl::CreateImage(const math::dimension2du& size, ColorFormat format, void* data, bool CopyMem, bool deleteOnDrop)
{
	StrongRef<Image> img = LUX_NEW(ImageImpl);
	img->Init(size, format, data, CopyMem, deleteOnDrop);
	return img;
}

void ImageSystemImpl::WriteImageToFile(Image* image, io::File* file)
{
	LX_CHECK_NULL_ARG(image);
	LX_CHECK_NULL_ARG(file);

	video::ImageLock lock(image);
	WriteImageDataToFile(image->GetSize(), image->GetColorFormat(), lock.data, file);
}

void ImageSystemImpl::WriteImageToFile(Image* image, const io::path& filePath)
{
	LX_CHECK_NULL_ARG(image);

	video::ImageLock lock(image);
	WriteImageDataToFile(image->GetSize(), image->GetColorFormat(), lock.data, filePath);
}

void ImageSystemImpl::WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, io::File* file)
{
	LX_CHECK_NULL_ARG(data);
	LX_CHECK_NULL_ARG(file);

	ImageWriter* writer = GetImageWriter(file->GetName());
	if(!writer)
		throw core::FileFormatException(io::GetFileExtension(file->GetName()).Data());

	writer->WriteFile(file, data, format, size, size.width * format.GetBytePerPixel());
}

void ImageSystemImpl::WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, const io::path& filePath)
{
	LX_CHECK_NULL_ARG(data);

	StrongRef<io::File> file = m_Filesystem->OpenFile(filePath, io::EFileMode::Write, true);
	WriteImageDataToFile(size, format, data, file);
}

void ImageSystemImpl::AddExternalImageWriter(ImageWriter* writer)
{
	LX_CHECK_NULL_ARG(writer);

	m_WriterList.PushBack(writer);
}

size_t ImageSystemImpl::GetImageWriterCount() const
{
	return m_WriterList.Size();
}

StrongRef<ImageWriter> ImageSystemImpl::GetImageWriter(size_t index)
{
	return m_WriterList.At(index);
}

StrongRef<ImageWriter> ImageSystemImpl::GetImageWriter(const io::path& name)
{
	if(m_WriterList.IsEmpty())
		return nullptr;

	for(size_t i = m_WriterList.Size(); i > 0; --i) {
		if(m_WriterList[i - 1]->CanWriteFile(name))
			return m_WriterList[i - 1];
	}

	return nullptr;
}

StrongRef<Texture> ImageSystemImpl::AddTexture(const string& name, const math::dimension2du& size, ColorFormat format, bool isDynamic)
{
	StrongRef<Texture> texture = CreateTexture(format, size, isDynamic, nullptr);

	m_ResourceSystem->AddResource(name, texture);

	return texture;
}

StrongRef<Texture> ImageSystemImpl::AddTexture(const string& name, Image* image, bool isDynamic)
{
	StrongRef<Texture> texture = CreateTexture(image, isDynamic);

	m_ResourceSystem->AddResource(name, texture);

	return texture;
}
StrongRef<video::Texture> ImageSystemImpl::AddChromaKeyedTexture(video::Image* image, video::Color key)
{
	LX_CHECK_NULL_ARG(image);

	auto texture = AddTexture(image->GetOrigin().str, image->GetSize(), video::ColorFormat::A8R8G8B8);
	{
		video::TextureLock texLock(texture, video::BaseTexture::ETLM_OVERWRITE);
		u32 real_key = ((u32)key & 0x00FFFFFF);

		video::ImageLock imgLock(image);
		u32* dst = (u32*)texLock.data;

		video::ColorFormat format = image->GetColorFormat();
		for(size_t i = 0; i < image->GetSizeInBytes(); i += image->GetBytesPerPixel()) {
			*dst = format.FormatToA8R8G8B8(imgLock.data + i);
			if((*dst & 0x00FFFFFF) == real_key)
				*dst = 0;
			++dst;
		}
	}

	return texture;
}

StrongRef<Texture> ImageSystemImpl::GetChromaKeyedTexture(const io::path& p, video::Color key)
{
	StrongRef<Image> image = m_ResourceSystem->GetResource(core::ResourceType::Image, p);
	return AddChromaKeyedTexture(image, key);
}

StrongRef<Texture> ImageSystemImpl::GetChromaKeyedTexture(const io::path& p, const math::vector2i& pos)
{
	StrongRef<Image> image = m_ResourceSystem->GetResource(core::ResourceType::Image, p);
	return AddChromaKeyedTexture(image, image->GetPixel(pos.x, pos.y));
}

StrongRef<CubeTexture> ImageSystemImpl::AddCubeTexture(const string& name, StrongRef<Image> images[6])
{
	StrongRef<video::CubeTexture> texture = CreateCubeTexture(images);
	m_ResourceSystem->AddResource(name, texture);
	return texture;
}

StrongRef<CubeTexture> ImageSystemImpl::AddCubeTexture(const string& name, ColorFormat format, u32 size)
{
	StrongRef<video::CubeTexture> texture = CreateCubeTexture(format, size);
	m_ResourceSystem->AddResource(name, texture);
	return texture;
}

StrongRef<Texture> ImageSystemImpl::AddRendertargetTexture(const string& name, const math::dimension2du& size, ColorFormat format)
{
	if(!m_Driver)
		throw core::Exception("No driver available");

	StrongRef<Texture> texture = m_Driver->CreateRendertargetTexture(size, format);
	m_ResourceSystem->AddResource(name, texture);
	m_Rendertargets.PushBack(texture);
	return texture;
}

StrongRef<SpriteBank> ImageSystemImpl::CreateSpriteBank()
{
	return LUX_NEW(video::SpriteBankImpl)(m_Driver);
}

bool ImageSystemImpl::SetTextureCreationFlag(ETextureCreationFlag Flag, bool Set)
{
	bool old = GetTextureCreationFlag(Flag);
	if(Set && ((Flag == ETCF_ALWAYS_16_BIT) || (Flag == ETCF_ALWAYS_32_BIT))) {
		SetTextureCreationFlag(ETCF_ALWAYS_16_BIT, false);
		SetTextureCreationFlag(ETCF_ALWAYS_32_BIT, false);
	}

	m_TextureCreationFlags = (m_TextureCreationFlags & (~Flag)) | ((((u32)!Set) - 1) & Flag);

	return old;
}

bool ImageSystemImpl::GetTextureCreationFlag(ETextureCreationFlag Flag) const
{
	return ((m_TextureCreationFlags & Flag) != 0);
}

}
}
