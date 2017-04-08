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
#include "ImageLoaderD3DX.h"
#include "ImageLoaderPNM.h"
#include "ImageLoaderTGA.h"
#include "ImageLoaderPNG.h"

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
		core::Name type = GetFileType(file);
		if(type == core::ResourceType::Image)
			return core::ResourceType::Texture;
		if(type == core::ResourceType::ImageList)
			return core::ResourceType::CubeTexture;

		return core::Name::INVALID;
	}

	bool LoadResource(io::File* file, core::Resource* dst)
	{
		StrongRef<core::Resource> r = m_ResourceSystem->GetResource(core::Name::INVALID, file);
		bool result;
		if(r) {
			if(r->GetReferableSubType() == core::ResourceType::Image) {
				StrongRef<Image> img = r;
				Texture* texture = dynamic_cast<Texture*>(dst);
				ColorFormat format = img->GetColorFormat();
				math::dimension2du size = img->GetDimension();
				result = m_ImageSystem->GetFittingTextureFormat(format, size, false);
				if(!result)
					return false;
				result = texture->Init(size, format, 0, false, false);
				if(!result)
					return false;
				void* imageBits = img->Lock();
				if(imageBits) {
					BaseTexture::SLockedRect rect;
					void* TexBits = texture->Lock(BaseTexture::ETLM_OVERWRITE, &rect);

					if(TexBits) {
						ColorConverter::ConvertByFormat(
							imageBits, img->GetColorFormat(),
							TexBits, texture->GetColorFormat(),
							img->GetDimension().width,
							img->GetDimension().height,
							0, rect.pitch);

						texture->Unlock();
						texture->RegenerateMIPMaps();
					}
				}

				img->Unlock();
				return result;
			}
		}

		return false;
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
public:
	MultiImageToCubeTextureLoader(core::ResourceSystem* resSys, ImageSystemImpl* imagSys) :
		m_ImageSystem(imagSys),
		m_ResSys(resSys)
	{
	}

	core::Name GetResourceType(io::File* file, core::Name requestedType)
	{
		if(requestedType && requestedType != core::ResourceType::CubeTexture)
			return core::Name::INVALID;

		auto fileSys = m_ImageSystem->m_Filesystem;
		auto filename = file->GetName();
		if(fileSys->ExistFile(filename)) {
			io::path basePath = io::GetFileDir(filename);
			string nameonly = io::GetFilenameOnly(filename, false);
			if(*nameonly.Last() != '1')
				return core::Name::INVALID;
			nameonly = nameonly.SubString(nameonly.First(), nameonly.Last());
			basePath += nameonly;
			string ext = io::GetFileExtension(filename);

			io::path p = basePath + core::StringConverter::ToString(2) + "." + ext;
			if(fileSys->ExistFile(p))
				return core::ResourceType::CubeTexture;
		}

		return core::Name::INVALID;
	}

	bool LoadResource(io::File* file, core::Resource* dst)
	{
		auto fileSys = m_ImageSystem->m_Filesystem;
		auto filename = file->GetName();
		io::path basePath = io::GetFileDir(filename);
		string nameonly = io::GetFilenameOnly(filename, false);
		nameonly = nameonly.SubString(nameonly.First(), nameonly.Last());
		basePath += nameonly;
		string ext = io::GetFileExtension(filename);

		bool isValid = true;
		io::path image_path[6];
		StrongRef<Image> images[6];
		for(size_t i = 0; i < 6; ++i) {
			image_path[i] = basePath + core::StringConverter::ToString(i + 1) + "." + ext;
			if(!fileSys->ExistFile(image_path[i]))
				isValid = false;
		}

		if(isValid) {
			for(size_t i = 0; i < 6; ++i)
				images[i] = m_ResSys->GetResource(core::ResourceType::Image, image_path[i]);
		} else {
			log::Error("Can't open file: ~s.", filename);
			return nullptr;
		}

		return m_ImageSystem->InitCubeTexture(images, (CubeTexture*)dst);
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

	if(m_Driver && m_Driver->GetVideoDriverType() == EVD_DIRECT9) {
		IDirect3DDevice9* d3dDevice = reinterpret_cast<IDirect3DDevice9*>(m_Driver->GetDevice());
		m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderD3DX)(d3dDevice));
	}

	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderBMP));
	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderPNM));
	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderTGA));
	m_ResourceSystem->AddResourceLoader(LUX_NEW(ImageLoaderPNG));

	AddExternalImageWriter(LUX_NEW(ImageWriterBMP));
	AddExternalImageWriter(LUX_NEW(ImageWriterTGA));

	m_ResourceSystem->GetReferableFactor()->RegisterType(LUX_NEW(video::ImageImpl));
	m_ResourceSystem->GetReferableFactor()->RegisterType(LUX_NEW(video::ImageListImpl));
}

ImageSystemImpl::~ImageSystemImpl()
{
}

StrongRef<Texture> ImageSystemImpl::CreateTexture(ColorFormat format, math::dimension2du size, bool isDynamic, void* imageBits)
{
	if(!m_Driver)
		return nullptr;

	ColorFormat srcFormat = format;
	if(imageBits) {
		if(GetFittingTextureFormat(format, size, false) == false) {
			log::Error("The texture format is not supported(~a).", format);
			return nullptr;
		}

		if(ColorConverter::IsConvertable(srcFormat, format) == false) {
			log::Error("The texture format is not supported(~a).", format);
			return nullptr;
		}
	}

	StrongRef<Texture> texture = m_Driver->CreateTexture(size, format,
		GetTextureCreationFlag(ETCF_CREATE_MIP_MAPS) ? 0 : 1,
		GetTextureCreationFlag(ETCF_ALPHA_CHANNEL), isDynamic);
	if(!texture)
		return nullptr;

	if(imageBits) {
		// Lock texture
		BaseTexture::SLockedRect rect;
		void* TexBits = texture->Lock(BaseTexture::ETLM_OVERWRITE, &rect);

		if(TexBits) {
			ColorConverter::ConvertByFormat(imageBits, srcFormat,
				TexBits, format,
				size.width, size.height, 0, rect.pitch);

			// Unlock texture
			texture->Unlock();
			texture->RegenerateMIPMaps();
		}
	}

	return texture;
}

StrongRef<Texture> ImageSystemImpl::CreateTexture(Image* image, bool isDynamic)
{
	StrongRef<Texture> out = CreateTexture(image->GetColorFormat(), image->GetDimension(), isDynamic, image->Lock());
	image->Unlock();
	return out;
}

StrongRef<CubeTexture> ImageSystemImpl::CreateCubeTexture(ColorFormat format, u32 size)
{
	if(!m_Driver)
		return nullptr;

	math::dimension2du tmp = math::dimension2du(size, size);
	if(GetFittingTextureFormat(format, tmp, true) == false) {
		log::Error("The texture format is not supported(~a).", format);
		return nullptr;
	}

	return m_Driver->CreateCubeTexture(tmp.width, format, GetTextureCreationFlag(ETCF_ALPHA_CHANNEL), false);
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
				size = images[i]->GetDimension().GetAvgEdge();
			}

			++real_count;
		}
	}

	if(real_count == 0) {
		return nullptr;
	}

	StrongRef<CubeTexture> texture = CreateCubeTexture(format, size);
	if(texture == nullptr)
		return nullptr;

	// Ein Face nach dem andererm sperren und füllen
	bool errorLog = false;
	for(u32 i = 0; i < 6; ++i) {
		BaseTexture::SLockedRect rect;
		void* data = texture->Lock(BaseTexture::ETLM_OVERWRITE, (CubeTexture::EFace)i, &rect);
		if(data) {
			void* imageBits = images[i] ? images[i]->Lock() : nullptr;
			if(imageBits) {
				ColorConverter::ConvertByFormat(imageBits, images[i]->GetColorFormat(),
					data, texture->GetColorFormat(),
					images[i]->GetDimension().width, images[i]->GetDimension().height, 0, rect.pitch);
				images[i]->Unlock();
			}

			texture->Unlock();
		} else {
			if(!errorLog)
				log::Error("Cant lock cube texture, for loading.");
			errorLog = true;
			return false;
		}
	}

	return texture;
}


bool ImageSystemImpl::InitCubeTexture(StrongRef<Image> images[6], CubeTexture* tex)
{
	ColorFormat format = ColorFormat::A8R8G8B8;
	u32 size = 0;
	u32 real_count = 0;
	for(u32 i = 0; i < 6; ++i) {
		if(images[i]) {
			if(real_count == 0) {
				format = images[i]->GetColorFormat();
				size = images[i]->GetDimension().GetAvgEdge();
			}

			++real_count;
		}
	}

	if(real_count == 0)
		return false;

	math::dimension2du ds(size, size);
	if(!GetFittingTextureFormat(format, ds, true)) {
		log::Error("The texture format is not supported(~a).", format);
		return false;
	}

	if(!tex->Init(size, format, false))
		return false;

	// Ein Face nach dem andererm sperren und füllen
	bool errorLog = false;
	for(u32 i = 0; i < 6; ++i) {
		BaseTexture::SLockedRect rect;
		void* data = tex->Lock(BaseTexture::ETLM_OVERWRITE, (CubeTexture::EFace)i, &rect);
		if(data) {
			void* imageBits = images[i] ? images[i]->Lock() : nullptr;
			if(imageBits) {
				ColorConverter::ConvertByFormat(imageBits, images[i]->GetColorFormat(),
					data, tex->GetColorFormat(),
					images[i]->GetDimension().width, images[i]->GetDimension().height, 0, rect.pitch);
				images[i]->Unlock();
			}

			tex->Unlock();
		} else {
			if(!errorLog)
				log::Error("Cant lock cube texture, for loading.");
			errorLog = true;
			return false;
		}
	}

	return true;
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
			bool alpha = GetTextureCreationFlag(ETCF_ALPHA_CHANNEL);
			if(m_Driver->CheckTextureFormat(format, alpha, cube))
				format = format;
			else if(m_Driver->CheckTextureFormat(ColorFormat::A8R8G8B8, alpha, cube))
				format = ColorFormat::A8R8G8B8;
			else if(m_Driver->CheckTextureFormat(ColorFormat::A1R5G5B5, alpha, cube))
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
			bool alpha = GetTextureCreationFlag(ETCF_ALPHA_CHANNEL);
			if(m_Driver->CheckTextureFormat(format, alpha, cube))
				format = format;
			else if(m_Driver->CheckTextureFormat(ColorFormat::A8R8G8B8, alpha, cube))
				format = ColorFormat::A8R8G8B8;
			else if(m_Driver->CheckTextureFormat(ColorFormat::R5G6B5, alpha, cube))
				format = ColorFormat::A1R5G5B5;
			else
				return false;
		}
	}

	return true;
}

StrongRef<Image> ImageSystemImpl::CreateImage(const math::dimension2du& size, ColorFormat format)
{
	if(format == ColorFormat::UNKNOWN)
		return nullptr;

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

bool ImageSystemImpl::WriteImageToFile(Image* image, io::File* file)
{
	bool ret = WriteImageDataToFile(image->GetDimension(), image->GetColorFormat(), image->Lock(), file);
	image->Unlock();
	return ret;
}

bool ImageSystemImpl::WriteImageToFile(Image* image, const io::path& filePath)
{
	bool ret;
	ret = WriteImageDataToFile(image->GetDimension(), image->GetColorFormat(), image->Lock(), filePath);
	image->Unlock();
	return ret;
}
bool ImageSystemImpl::WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, io::File* file)
{
	ImageWriter* writer = GetImageWriter(file->GetName());
	if(writer) {
		bool result = writer->WriteFile(file, data, format, size, size.width * format.GetBytePerPixel());
		if(!result)
			log::Error("Can't write image file.");

		return result;
	} else {
		log::Error("Can't write image in format: ~s.", io::GetFileExtension(file->GetName()));
		return false;
	}
}

bool ImageSystemImpl::WriteImageDataToFile(const math::dimension2du& size, ColorFormat format, void* data, const io::path& filePath)
{
	ImageWriter* writer = GetImageWriter(filePath);
	if(writer) {
		StrongRef<io::File> file = m_Filesystem->OpenFile(filePath, io::EFileMode::Write, true);
		if(!file) {
			log::Error("Can't open file: ~s.", filePath);
			return false;
		}
		bool result = writer->WriteFile(file, data, format, size, size.width * format.GetBytePerPixel());
		if(!result)
			log::Error("Can't write image file: ~s.", filePath);

		return result;
	} else {
		log::Error("Can't write image in format: ~s.", io::GetFileExtension(filePath));
		return false;
	}
}

void ImageSystemImpl::AddExternalImageWriter(ImageWriter* writer)
{
	if(writer)
		m_WriterList.Push_Back(writer);
}

size_t ImageSystemImpl::GetImageWriterCount() const
{
	return m_WriterList.Size();
}

ImageWriter* ImageSystemImpl::GetImageWriter(size_t index)
{
	if(index >= m_WriterList.Size())
		return nullptr;
	return m_WriterList[index];
}

ImageWriter* ImageSystemImpl::GetImageWriter(const io::path& name)
{
	if(m_WriterList.IsEmpty())
		return nullptr;

	for(size_t i = m_WriterList.Size() - 1; i >= 0; --i) {
		if(m_WriterList[i]->CanWriteFile(name))
			return m_WriterList[i];
	}

	return nullptr;
}

Texture* ImageSystemImpl::AddTexture(const string& name, const math::dimension2du& size, ColorFormat format, bool isDynamic)
{
	StrongRef<Texture> texture = CreateTexture(format, size, isDynamic, nullptr);

	if(texture) {
		if(!m_ResourceSystem->AddResource(name, texture))
			return nullptr;
	}

	return texture;
}

Texture* ImageSystemImpl::AddTexture(const string& name, Image* image, bool isDynamic)
{
	StrongRef<Texture> texture = CreateTexture(image, isDynamic);

	if(texture) {
		if(!m_ResourceSystem->AddResource(name, texture))
			return nullptr;
	}

	return texture;
}

CubeTexture* ImageSystemImpl::AddCubeTexture(const string& name, StrongRef<Image> images[6])
{
	StrongRef<video::CubeTexture> texture = CreateCubeTexture(images);

	if(texture) {
		if(!m_ResourceSystem->AddResource(name, texture))
			return nullptr;
	}

	return texture;
}


CubeTexture* ImageSystemImpl::AddCubeTexture(const string& name, ColorFormat format, u32 size)
{
	StrongRef<video::CubeTexture> texture = CreateCubeTexture(format, size);

	if(texture) {
		if(!m_ResourceSystem->AddResource(name, texture))
			return nullptr;
	}

	return texture;
}

Texture* ImageSystemImpl::AddRendertargetTexture(const string& name, const math::dimension2du& size, ColorFormat format)
{
	if(!m_Driver)
		return nullptr;

	StrongRef<Texture> texture = m_Driver->CreateRendertargetTexture(size, format, true);

	if(texture) {
		if(!m_ResourceSystem->AddResource(name, texture))
			return nullptr;
		m_Rendertargets.Push_Back(texture);
	}

	return texture;
}

StrongRef<SpriteBank> ImageSystemImpl::CreateSpriteBank()
{
	return LUX_NEW(video::SpriteBankImpl)(m_Driver);
}

bool ImageSystemImpl::SetTextureCreationFlag(ETextureCreationFlag Flag, bool Set)
{
	bool Old = GetTextureCreationFlag(Flag);
	if(Set && ((Flag == ETCF_ALWAYS_16_BIT) || (Flag == ETCF_ALWAYS_32_BIT))) {
		SetTextureCreationFlag(ETCF_ALWAYS_16_BIT, false);
		SetTextureCreationFlag(ETCF_ALWAYS_32_BIT, false);
	}

	m_TextureCreationFlags = (m_TextureCreationFlags & (~Flag)) | ((((u32)!Set) - 1) & Flag);

	return Old;
}

bool ImageSystemImpl::GetTextureCreationFlag(ETextureCreationFlag Flag) const
{
	return ((m_TextureCreationFlags & Flag) != 0);
}

}
}