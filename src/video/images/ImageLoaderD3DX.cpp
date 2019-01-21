#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER

#include "ImageLoaderD3DX.h"
#include "io/File.h"
#include "video/ColorConverter.h"
#include "video/images/Image.h"
#include "video/Texture.h"
#include "core/lxMemory.h"
#include "platform/StrippedD3D9X.h"
#include "platform/UnknownRefCounted.h"
#include "video/d3d9/D3DHelper.h"

namespace lux
{
namespace video
{

ImageLoaderD3DX::ImageLoaderD3DX(IDirect3DDevice9* device) :
	m_Device(device)
{
}

ImageLoaderD3DX::~ImageLoaderD3DX()
{
}

core::Name ImageLoaderD3DX::GetResourceType(io::File* file, core::Name requestedType)
{
	if(!requestedType.IsEmpty() && requestedType != core::ResourceType::Image &&
		requestedType != core::ResourceType::Texture)
		return core::Name::INVALID;

	core::String ext = file->GetPath().GetFileExtension();
	u8 bytes[128];
	auto count = file->ReadBinaryPart(sizeof(bytes), bytes);
	if(count >= 2 && bytes[0] == 0xFF && bytes[1] == 0xD8)
		return requestedType; // jpg
	if(count >= 128 && // 128 ist the 4 bytes + the size of the DDS header
		bytes[0] == 'D' && bytes[1] == 'D' &&
		bytes[2] == 'S' && bytes[3] == ' ') {
		// dds

		// Check pixel format to determine type.
		u8* pxformat = bytes + 76;
		u8* flagptr = pxformat + 4;
		u32 flags = (flagptr[3] << 24) | (flagptr[2] << 16) | (flagptr[1] << 8) | flagptr[0];
		if(flags & 0x4) {
			// Compressed data, can only be loaded as texture
			if(!requestedType.IsEmpty() && requestedType != core::ResourceType::Texture)
				return core::Name::INVALID;
			return core::ResourceType::Texture;
		}
		// Otherwise we don't care
		return core::ResourceType::Image;
	}
	return core::Name::INVALID;
}

const core::String& ImageLoaderD3DX::GetName() const
{
	static const core::String name = "Lux D3DX loader proxy";

	return name;
}

static ColorFormat ConvertD3DToLuxFormat(D3DFORMAT Format)
{
	switch(Format) {
	case D3DFMT_R8G8B8:
		return ColorFormat::R8G8B8;
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8R8G8B8:
		return ColorFormat::A8R8G8B8;
	case D3DFMT_A1R5G5B5:
		return ColorFormat::A1R5G5B5;
	case D3DFMT_R5G6B5:
		return ColorFormat::R5G6B5;
	case D3DFMT_DXT1:
		return ColorFormat::DXT1;
	case D3DFMT_DXT2:
		return ColorFormat::A8R8G8B8;
	case D3DFMT_DXT3:
		return ColorFormat::DXT3;
	case D3DFMT_DXT4:
		return ColorFormat::A8R8G8B8;
	case D3DFMT_DXT5:
		return ColorFormat::DXT5;
	default:
		return ColorFormat::UNKNOWN;
	}
}

static UnknownRefCounted<IDirect3DBaseTexture9> LoadTexture(
	IDirect3DDevice9* device,
	void* buffer, int bufferSize,
	D3DFORMAT format,
	const D3DXIMAGE_INFO& imageInfo,
	D3DSURFACE_DESC& outDesc)
{
	HRESULT hr = E_FAIL;
	switch(imageInfo.ResourceType) {
	case D3DRTYPE_TEXTURE:
	{
		UnknownRefCounted<IDirect3DTexture9> out;
		hr = D3DXLibraryLoader::Instance().GetD3DXCreateTextureFromFileInMemoryEx()(device,
			buffer, bufferSize,
			D3DX_DEFAULT, D3DX_DEFAULT,
			1, 0, format, D3DPOOL_SYSTEMMEM,
			D3DX_DEFAULT, D3DX_DEFAULT, 0,
			nullptr, nullptr, out.Access());
		if(FAILED(hr))
			return nullptr;

		out->GetLevelDesc(0, &outDesc);

		return (IDirect3DBaseTexture9*)out;
	}
	break;
	case D3DRTYPE_CUBETEXTURE:
	{
		UnknownRefCounted<IDirect3DCubeTexture9> out;
		hr = D3DXLibraryLoader::Instance().GetD3DXCreateCubeTextureFromFileInMemoryEx()(device,
			buffer, bufferSize,
			D3DX_DEFAULT, 1, 0,
			format, D3DPOOL_SYSTEMMEM,
			D3DX_DEFAULT, D3DX_DEFAULT,
			0, nullptr, nullptr, out.Access());
		if(FAILED(hr))
			return nullptr;
		out->GetLevelDesc(0, &outDesc);
		return (IDirect3DBaseTexture9*)out;
	}
	break;
	case D3DRTYPE_VOLUMETEXTURE:
	{
		UnknownRefCounted<IDirect3DVolumeTexture9> out;
		hr = D3DXLibraryLoader::Instance().GetD3DXCreateVolumeTextureFromFileInMemoryEx()(device,
			buffer, bufferSize,
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			1, 0, format, D3DPOOL_SYSTEMMEM,
			D3DX_DEFAULT, D3DX_DEFAULT,
			0, nullptr, nullptr, out.Access());

		if(FAILED(hr))
			return nullptr;
		D3DVOLUME_DESC desc;
		out->GetLevelDesc(0, &desc);
		outDesc.Format = desc.Format;
		outDesc.Height = desc.Height;
		outDesc.Pool = desc.Pool;
		outDesc.Type = desc.Type;
		outDesc.Usage = desc.Usage;
		outDesc.Width = desc.Width;
		return (IDirect3DBaseTexture9*)out;
	}
	break;
	default:
		return nullptr;
	}
}

static void ConvertLine(void* Src, void* Dst, D3DFORMAT SrcFormat, ColorFormat DstFormat, int Width)
{
	if(SrcFormat == D3DFMT_R8G8B8 && DstFormat == ColorFormat::R8G8B8)
		ColorConverter::ConvertByFormat(Src, ColorFormat::R8G8B8, Dst, DstFormat, Width, 1);
	if(SrcFormat == D3DFMT_A8R8G8B8)
		ColorConverter::ConvertByFormat(Src, ColorFormat::A8R8G8B8, Dst, DstFormat, Width, 1);
	if(SrcFormat == D3DFMT_X8R8G8B8) {
		ColorConverter::ConvertByFormat(Src, ColorFormat::A8R8G8B8, Dst, DstFormat, Width, 1);
		//Dst format is always R8G8B8 oder A8R8G8B8
		// Alpha nur bei letzerem wichtig
		if(DstFormat == ColorFormat::A8R8G8B8) {
			u8* dB = (u8*)Dst;
			for(int i = 0; i < Width; ++i) {
				dB[3] = 0xFF;
				dB += 4;
			}
		}
	}
}

static void CopyTextureData(
	u32 imageIndex, void* dest,
	IDirect3DBaseTexture9* base_texture,
	const D3DXIMAGE_INFO& info,
	const D3DSURFACE_DESC& desc,
	video::ColorFormat lxFormat)
{
	switch(info.ResourceType) {
	case D3DRTYPE_TEXTURE:
	{
		D3DLOCKED_RECT lock;
		IDirect3DTexture9* texture = (IDirect3DTexture9*)base_texture;
		texture->LockRect(0, &lock, nullptr, D3DLOCK_READONLY);
		for(UINT i = 0; i < desc.Height; ++i) {
			ConvertLine(lock.pBits, dest, desc.Format, lxFormat, desc.Width);
			dest = (u8*)dest + lxFormat.GetBytePerPixel() * desc.Width;
			lock.pBits = (u8*)lock.pBits + lock.Pitch;
		}
		texture->UnlockRect(0);
	}
	break;

	case D3DRTYPE_CUBETEXTURE:
	{
		D3DLOCKED_RECT lock;
		IDirect3DCubeTexture9* texture = (IDirect3DCubeTexture9*)base_texture;
		texture->LockRect((D3DCUBEMAP_FACES)imageIndex, 0, &lock, nullptr, D3DLOCK_READONLY);
		for(UINT i = 0; i < desc.Height; ++i) {
			ConvertLine(lock.pBits, dest, desc.Format, lxFormat, desc.Width);
			dest = (u8*)dest + lxFormat.GetBytePerPixel() * desc.Width;
			lock.pBits = (u8*)lock.pBits + lock.Pitch;
		}
		texture->UnlockRect((D3DCUBEMAP_FACES)imageIndex, 0);
	}
	break;

	case D3DRTYPE_VOLUMETEXTURE:
	{
		IDirect3DVolumeTexture9* texture = (IDirect3DVolumeTexture9*)base_texture;
		D3DLOCKED_BOX lock;
		D3DBOX box;
		box.Left = 0;
		box.Top = 0;
		box.Right = desc.Width;
		box.Bottom = desc.Height;
		box.Front = imageIndex;
		box.Back = imageIndex + 1;
		HRESULT hr = texture->LockBox(0, &lock, &box, D3DLOCK_READONLY);
		if(SUCCEEDED(hr)) {
			for(UINT i = 0; i < desc.Height; ++i) {
				ConvertLine(lock.pBits, dest, desc.Format, lxFormat, desc.Width);
				dest = (u8*)dest + lxFormat.GetBytePerPixel() * desc.Width;
				lock.pBits = (u8*)lock.pBits + lock.RowPitch;
			}
			texture->UnlockBox(0);
		}
	}
	break;
	default:
		lxAssertNeverReach("Unsupported texture type.");
	}
}

struct TempMemory
{
	u8* ptr;
	size_t size;
	bool drop;

	~TempMemory()
	{
		if(drop)
			LUX_FREE_ARRAY(ptr);
	}
};

void ImageLoaderD3DX::LoadResource(io::File* file, core::Resource* dst)
{
	HRESULT hr;

	TempMemory buffer;

	buffer.size = core::SafeCast<size_t>(file->GetSize() - file->GetCursor());
	if(file->GetBuffer() == nullptr) {
		u8* tmp = LUX_NEW_ARRAY(u8, buffer.size);
		buffer.ptr = tmp;
		buffer.drop = true;
		file->ReadBinary(buffer.size, tmp);
	} else {
		buffer.ptr = (u8*)file->GetBuffer();
		buffer.drop = false;
	}

	if(!buffer.ptr)
		throw core::FileFormatException("Can't loader data from file", "<unknown>");

	D3DXIMAGE_INFO info;
	hr = D3DXLibraryLoader::Instance().GetD3DXGetImageInfoFromFileInMemory()(buffer.ptr, (UINT)buffer.size, &info);
	if(FAILED(hr))
		throw core::FileFormatException("Corrupted or not supported", file->GetPath().GetFileExtension());

	auto loadFormat = GetD3DFormat(ConvertD3DToLuxFormat(info.Format));
	D3DSURFACE_DESC desc;
	auto d3dTexture = LoadTexture(
		m_Device,
		buffer.ptr, core::SafeCast<UINT>(buffer.size),
		loadFormat, info,
		desc);

	if(!d3dTexture)
		throw core::FileFormatException("Corrupted or not supported", file->GetPath().GetFileExtension());

	auto lxFormat = ConvertD3DToLuxFormat(desc.Format);
	if(lxFormat == video::ColorFormat::UNKNOWN)
		throw core::FileFormatException("Unsupported color format", file->GetPath().GetFileExtension());

	video::Image* img = dynamic_cast<video::Image*>(dst);
	if(img) {
		img->Init(math::Dimension2I(desc.Width, desc.Height), lxFormat);

		video::ImageLock imgLock(img);
		CopyTextureData(0, imgLock.data, d3dTexture, info, desc, lxFormat);
		return;
	}

	video::Texture* texture = dynamic_cast<video::Texture*>(dst);
	if(texture) {
		texture->Init(math::Dimension2I(desc.Width, desc.Height), lxFormat, 0, false, false);

		if(info.ResourceType == D3DRTYPE_TEXTURE) {
			video::TextureLock texLock(texture, BaseTexture::ELockMode::Overwrite);
			D3DLOCKED_RECT lock;
			auto orgTex = d3dTexture.StaticCast<IDirect3DTexture9>();
			orgTex->LockRect(0, &lock, nullptr, D3DLOCK_READONLY);
			if(lxFormat.IsCompressed())
				memcpy(texLock.data, lock.pBits, (desc.Width*desc.Height*lxFormat.GetBitsPerPixel()) / 8);
			else
				memcpy(texLock.data, lock.pBits, desc.Height*lock.Pitch);
			orgTex->UnlockRect(0);
			return;
		} else {
			throw core::FileFormatException("Can't load cube- and volumetexture as texture", "<unknown>");
		}
	}
}

}
}

#endif // LUX_COMPILE_WITH_D3DX_IMAGE_LOADER