#include "ImageLoaderD3DX.h"
#include "io/File.h"
#include "video/ColorConverter.h"
#include "video/images/Image.h"
#include "core/lxMemory.h"
#include "StrippedD3D9X.h"

namespace lux
{
namespace video
{

ImageLoaderD3DX::ImageLoaderD3DX(IDirect3DDevice9* pDevice) :
	m_Device(pDevice)
{
}

ImageLoaderD3DX::~ImageLoaderD3DX()
{
}

core::Name ImageLoaderD3DX::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Image)
		return core::Name::INVALID;

	string ext = io::GetFileExtension(file->GetName());
	if(ext.EqualCaseInsensitive("jpg"))
		return core::ResourceType::Image;

	return core::Name::INVALID;
}

const string& ImageLoaderD3DX::GetName() const
{
	static const string name = "Lux D3DX loader proxy";

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
	default:
		return ColorFormat::UNKNOWN;
	}
}

static bool LoadTexture(
	IDirect3DDevice9* device,
	void* buffer, size_t bufferSize,
	D3DFORMAT format,
	const D3DXIMAGE_INFO& imageInfo,
	IDirect3DBaseTexture9*& outTexture,
	D3DSURFACE_DESC& outDesc)
{
	HRESULT hr = E_FAIL;
	switch(imageInfo.ResourceType) {
	case D3DRTYPE_TEXTURE:
	{
		IDirect3DTexture9* Tex;
		hr = D3DXCreateTextureFromFileInMemoryEx(device,
			buffer, (UINT)bufferSize,
			D3DX_DEFAULT, D3DX_DEFAULT,
			1, 0, format, D3DPOOL_SYSTEMMEM,
			D3DX_DEFAULT, D3DX_DEFAULT, 0,
			nullptr, nullptr, &Tex);
		if(FAILED(hr))
			return false;

		Tex->GetLevelDesc(0, &outDesc);

		outTexture = Tex;
	}
	break;
	case D3DRTYPE_CUBETEXTURE:
	{
		IDirect3DCubeTexture9* Tex;
		hr = D3DXCreateCubeTextureFromFileInMemoryEx(device,
			buffer, (UINT)bufferSize,
			D3DX_DEFAULT, 1, 0,
			format, D3DPOOL_SYSTEMMEM,
			D3DX_DEFAULT, D3DX_DEFAULT,
			0, nullptr, nullptr, &Tex);
		if(FAILED(hr))
			return false;
		Tex->GetLevelDesc(0, &outDesc);
		outTexture = Tex;
	}
	break;
	case D3DRTYPE_VOLUMETEXTURE:
	{
		IDirect3DVolumeTexture9* tex;
		hr = D3DXCreateVolumeTextureFromFileInMemoryEx(device,
			buffer, (UINT)bufferSize,
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			1, 0, format, D3DPOOL_SYSTEMMEM,
			D3DX_DEFAULT, D3DX_DEFAULT,
			0, nullptr, nullptr, &tex);

		if(FAILED(hr))
			return false;
		D3DVOLUME_DESC desc;
		tex->GetLevelDesc(0, &desc);
		outDesc.Format = desc.Format;
		outDesc.Height = desc.Height;
		outDesc.Pool = desc.Pool;
		outDesc.Type = desc.Type;
		outDesc.Usage = desc.Usage;
		outDesc.Width = desc.Width;
		outTexture = tex;
	}
	break;
	default:
		return false;
	}

	return true;
}

static void ConvertLine(void* Src, void* Dst, D3DFORMAT SrcFormat, ColorFormat DstFormat, u32 Width)
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
			for(u32 i = 0; i < Width; ++i) {
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
		for(int i = 0; i < (int)desc.Height; ++i) {
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
		for(int i = 0; i < (int)desc.Height; ++i) {
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
			for(int i = 0; i < (int)desc.Height; ++i) {
				ConvertLine(lock.pBits, dest, desc.Format, lxFormat, desc.Width);
				dest = (u8*)dest + lxFormat.GetBytePerPixel() * desc.Width;
				lock.pBits = (u8*)lock.pBits + lock.RowPitch;
			}
			texture->UnlockBox(0);
		}
	}
	break;
	default:
		assertNeverReach("Unsupported texture type.");
	}
}

struct TempMemory
{
	void* ptr;
	size_t size;
	bool drop;

	~TempMemory()
	{
		if(drop)
			delete ptr;
	}
};

bool ImageLoaderD3DX::LoadResource(io::File* file, core::Resource* dst)
{
	HRESULT hr;

	TempMemory buffer;

	buffer.size = file->GetSize() - file->GetCursor();
	if(file->GetBuffer() == nullptr) {
		void* tmp = LUX_NEW_ARRAY(u8, buffer.size);
		size_t read = file->ReadBinary((u32)buffer.size, tmp);
		if(read != buffer.size)
			return false;
		buffer.ptr = tmp;
		buffer.drop = true;
	} else {
		buffer.ptr = file->GetBuffer();
		buffer.drop = false;
	}

	if(!buffer.ptr)
		return false;

	D3DXIMAGE_INFO info;
	hr = D3DXGetImageInfoFromFileInMemory(buffer.ptr, (UINT)buffer.size, &info);
	if(FAILED(hr))
		return false;

	IDirect3DBaseTexture9* texture;
	D3DSURFACE_DESC desc;
	bool result = LoadTexture(
		m_Device,
		buffer.ptr, buffer.size,
		info.Format, info,
		texture, desc);

	if(!result)
		return false;

	video::ColorFormat lxFormat = ConvertD3DToLuxFormat(desc.Format);
	if(lxFormat == video::ColorFormat::UNKNOWN)
		return false;
	video::Image* img = dynamic_cast<video::Image*>(dst);
	img->Init(
		math::dimension2du(desc.Width, desc.Height), lxFormat);
	void* imgData = img->Lock();
	CopyTextureData(0, imgData, texture, info, desc, lxFormat);
	img->Unlock();

	texture->Release();

	return true;
}


}

}

