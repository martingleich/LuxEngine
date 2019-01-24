#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "TextureD3D9.h"

#include "platform/StrippedD3D9X.h"
#include "video/d3d9/D3DHelper.h"
#include "platform/D3D9Exception.h"
#include "video/d3d9/AuxiliaryTextureD3D9.h"

namespace lux
{
namespace video
{
TextureD3D9::TextureD3D9(IDirect3DDevice9* device) :
	m_Device(device)
{
	LX_CHECK_NULL_ARG(device);
}

TextureD3D9::~TextureD3D9()
{
}

void TextureD3D9::Init(
	const math::Dimension2I& size,
	video::ColorFormat lxFormat,
	int mipCount, bool isRendertarget, bool isDynamic)
{
	if(m_Texture)
		m_Texture = nullptr;

	if(mipCount == 0)
		mipCount = math::HighestBitPos(math::Max(size.width, size.height));

	D3DFORMAT format = GetD3DFormat(lxFormat);
	if(format == D3DFMT_UNKNOWN)
		throw core::UnsupportedColorFormatException(lxFormat);

	// TODO: Check if a origin is available for reload.
	DWORD usage;
	D3DPOOL pool;
	if(isDynamic) {
		usage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	} if(isRendertarget) {
		usage = D3DUSAGE_RENDERTARGET;
		pool = D3DPOOL_DEFAULT;
	} else {
		usage = 0;
		pool = D3DPOOL_MANAGED;
	}

	HRESULT hr = m_Device->CreateTexture((UINT)size.width, (UINT)size.height, mipCount,
		usage, format,
		pool, m_Texture.Access(), nullptr);

	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	m_LockedLevels = 0;

	m_Usage = usage;
	m_Pool = pool;
	m_D3DFormat = format;
	m_Levels = mipCount;
	m_Size = size;
	m_Format = lxFormat;
}

BaseTexture::LockedRect TextureD3D9::Lock(ELockMode mode, int mipLevel)
{
	if(m_LockedLevels&((u32)1 << mipLevel))
		throw core::InvalidOperationException("Texture is already locked");
	if(mipLevel > m_Levels)
		throw core::InvalidOperationException("Invalid mip level");

	D3DLOCKED_RECT d3dlocked;
	DWORD flags;
	HRESULT hr;
	if(mode == ELockMode::Overwrite && m_Usage == D3DUSAGE_DYNAMIC)
		flags = D3DLOCK_DISCARD;
	else if(mode == ELockMode::ReadOnly)
		flags = D3DLOCK_READONLY;
	else
		flags = 0;

	hr = m_Texture->LockRect(mipLevel, &d3dlocked, nullptr, flags);

	if(FAILED(hr)) {
		if(mode == ELockMode::Overwrite && m_Usage == 0) {
			m_TempSurface = AuxiliaryTextureManagerD3D9::Instance()->GetSurface(DWORD(m_Size.width), DWORD(m_Size.height), m_D3DFormat);
			if(m_TempSurface)
				hr = m_TempSurface->LockRect(&d3dlocked, nullptr, D3DLOCK_DISCARD);
			if(FAILED(hr)) {
				m_TempSurface = nullptr;
				throw core::D3D9Exception(hr);
			}
		} else if(mode == ELockMode::ReadOnly && m_Usage == 0) {
			throw core::InvalidOperationException("Can't lock static texture in read mode");
		} else if(mode == ELockMode::ReadWrite && m_Usage == 0) {
			throw core::InvalidOperationException("Can't lock static texture in read mode");
		} else if(mode == ELockMode::ReadOnly && m_Usage == D3DUSAGE_RENDERTARGET) {
			m_TempSurface = AuxiliaryTextureManagerD3D9::Instance()->GetSurface(m_Size.width, m_Size.height, m_D3DFormat);
			if(m_TempSurface) {
				UnknownRefCounted<IDirect3DSurface9> surface;
				hr = m_Texture->GetSurfaceLevel(mipLevel, surface.Access());
				if(SUCCEEDED(hr))
					hr = m_Device->GetRenderTargetData(surface, m_TempSurface);
				if(SUCCEEDED(hr))
					hr = m_TempSurface->LockRect(&d3dlocked, nullptr, D3DLOCK_READONLY);
			}
			if(FAILED(hr)) {
				m_TempSurface = nullptr;
				throw core::D3D9Exception(hr);
			}
		}
	}

	m_LockedLevels |= u32(1) << mipLevel;

	LockedRect locked;
	locked.bits = (u8*)d3dlocked.pBits;
	locked.pitch = d3dlocked.Pitch;

	return locked;
}

void TextureD3D9::Unlock(bool regenMipMaps, int mipLevel)
{
	if((m_LockedLevels & (u32(1) << mipLevel)) == 0)
		return;

	HRESULT hr;
	if(m_TempSurface) {
		m_TempSurface->UnlockRect();
		if(m_Usage == 0) {
			UnknownRefCounted<IDirect3DSurface9> surface;
			hr = m_Texture->GetSurfaceLevel(mipLevel, surface.Access());
			if(SUCCEEDED(hr))
				hr = m_Device->UpdateSurface(m_TempSurface, nullptr, surface, nullptr);
			if(FAILED(hr))
				throw core::D3D9Exception(hr);
		}

		m_TempSurface = nullptr;
	} else {
		m_Texture->UnlockRect(mipLevel);
	}

	m_LockedLevels &= ~(u32(1) << mipLevel);
	if(regenMipMaps && m_Levels > 1) {
		// Regenerate mip maps
		if(m_Format.IsCompressed())
			return;
		hr = D3DXLibraryLoader::Instance().GetD3DXFilterTexture()(m_Texture, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
		if(FAILED(hr))
			throw core::D3D9Exception(hr);
	}
}

void TextureD3D9::ReleaseUnmanaged()
{
	if(m_Pool == D3DPOOL_MANAGED)
		return;
	m_Texture = nullptr;
}

void TextureD3D9::RestoreUnmanaged()
{
	if(m_Pool == D3DPOOL_MANAGED)
		return;

	Init(m_Size, m_Format, GetMipMapCount(), IsRendertarget(), IsDynamic());
}

}
}

#endif // LUX_COMPILE_WITH_D3D9
