#ifdef LUX_COMPILE_WITH_D3D9
#include "TextureD3D9.h"

#include "StrippedD3D9X.h"
#include "D3DHelper.h"
#include "D3D9Exception.h"

namespace lux
{
namespace video
{
u32 TextureD3D9::s_TextureCount = 0;
core::array<IDirect3DSurface9*> TextureD3D9::s_TempSurfaces;

TextureD3D9::TextureD3D9(IDirect3DDevice9* device) :
	m_Texture(nullptr),
	m_LockedLevel(0),
	m_IsLocked(false),
	m_TempSurface(nullptr),
	m_Device(device)
{
	++s_TextureCount;
}

TextureD3D9::~TextureD3D9()
{
	if(m_Texture)
		m_Texture->Release();
	--s_TextureCount;

	if(s_TextureCount == 0) {
		for(auto it = s_TempSurfaces.First(); it != s_TempSurfaces.End(); ++it) {
			(*it)->Release();
		}
		s_TempSurfaces.Clear();
	}
}

void TextureD3D9::RegenerateMIPMaps()
{
	HRESULT hr = D3DXFilterTexture(m_Texture, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
	if(FAILED(hr))
		throw core::D3D9Exception(hr);
}

void TextureD3D9::Init(
	const math::dimension2du& Size,
	video::ColorFormat lxFormat,
	u32 MipCount, bool isRendertarget, bool isDynamic)
{
	if(!m_Device)
		throw core::Exception("No driver available");

	if(m_Texture)
		m_Texture->Release();

	D3DFORMAT format = GetD3DFormat(lxFormat, lxFormat.HasAlpha());
	if(format == D3DFMT_UNKNOWN)
		throw core::ColorFormatException(lxFormat);

	UINT Levels = MipCount;
	if(MipCount == 0)
		Levels = 0;

	DWORD usage = 0;
	D3DPOOL pool = D3DPOOL_MANAGED;
	if(isDynamic) {
		usage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	}
	if(isRendertarget) {
		usage = D3DUSAGE_RENDERTARGET;
		pool = D3DPOOL_DEFAULT;
	}

	HRESULT hr = m_Device->CreateTexture(Size.width, Size.height, Levels,
		usage, format,
		D3DPOOL_DEFAULT, &m_Texture, nullptr);

	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	m_IsLocked = false;

	m_Texture->GetLevelDesc(0, &m_Desc);

	m_Dimension.Set(m_Desc.Width, m_Desc.Height);
	m_Format = lxFormat;
}

BaseTexture::LockedRect TextureD3D9::Lock(ETextureLockMode Mode, u32 MipLevel)
{
	if(m_IsLocked)
		throw core::Exception("Texture is already locked");

	m_LockedLevel = MipLevel;
	if(m_LockedLevel >= m_Texture->GetLevelCount())
		m_LockedLevel = m_Texture->GetLevelCount() - 1;

	D3DLOCKED_RECT Locked;
	DWORD Flags;
	HRESULT hr;
	if(Mode == ETLM_OVERWRITE && m_Desc.Usage == D3DUSAGE_DYNAMIC)
		Flags = D3DLOCK_DISCARD;
	else if(Mode == ETLM_READ_ONLY)
		Flags = D3DLOCK_READONLY;
	else
		Flags = 0;

	hr = m_Texture->LockRect(MipLevel, &Locked, nullptr, Flags);

	if(FAILED(hr)) {
		if(Mode == ETLM_OVERWRITE && m_Desc.Usage == 0) {
			m_TempSurface = GetTempSurface(m_Desc.Width, m_Desc.Height, m_Desc.Format);
			if(m_TempSurface)
				hr = m_TempSurface->LockRect(&Locked, nullptr, D3DLOCK_DISCARD);
			if(FAILED(hr)) {
				FreeTempSurface(m_TempSurface);
				throw core::D3D9Exception(hr);
			}
		} else if(Mode == ETLM_READ_ONLY && m_Desc.Usage == 0) {
			throw core::Exception("Can't lock static texture in read mode");
		} else if(Mode == ETLM_READ_WRITE && m_Desc.Usage == 0) {
			throw core::Exception("Can't lock static texture in read mode");
		} else if(Mode == ETLM_READ_ONLY && m_Desc.Usage == D3DUSAGE_RENDERTARGET) {
			m_TempSurface = GetTempSurface(m_Desc.Width, m_Desc.Height, m_Desc.Format);
			if(m_TempSurface) {
				IDirect3DDevice9* device;
				IDirect3DSurface9* surface;
				m_Texture->GetDevice(&device);
				m_Texture->GetSurfaceLevel(m_LockedLevel, &surface);
				device->GetRenderTargetData(surface, m_TempSurface);
				hr = m_TempSurface->LockRect(&Locked, nullptr, D3DLOCK_READONLY);
			}
			if(FAILED(hr)) {
				FreeTempSurface(m_TempSurface);
				throw core::D3D9Exception(hr);
			}
		}
	}

	LockedRect locked;
	locked.bits = (u8*)Locked.pBits;
	locked.pitch = Locked.Pitch;
	m_IsLocked = true;

	return locked;
}

void TextureD3D9::Unlock()
{
	if(!m_IsLocked)
		return;

	HRESULT hr;
	if(m_TempSurface) {
		m_TempSurface->UnlockRect();
		if(m_Desc.Usage == 0) {
			IDirect3DDevice9* device;
			IDirect3DSurface9* surface;
			m_Texture->GetDevice(&device);
			m_Texture->GetSurfaceLevel(m_LockedLevel, &surface);

			hr = device->UpdateSurface(m_TempSurface, nullptr, surface, nullptr);
			if(FAILED(hr))
				throw core::D3D9Exception(hr);
		}

		FreeTempSurface(m_TempSurface);
		m_TempSurface = nullptr;
	} else {
		m_Texture->UnlockRect(m_LockedLevel);
	}

	m_IsLocked = false;
}

bool TextureD3D9::IsRendertarget() const
{
	return (m_Desc.Usage == D3DUSAGE_RENDERTARGET);
}

ColorFormat TextureD3D9::GetColorFormat() const
{
	return m_Format;
}

void* TextureD3D9::GetRealTexture()
{
	return (void*)(m_Texture);
}

u32 TextureD3D9::GetLevelCount() const
{
	return m_Texture->GetLevelCount();
}

const math::dimension2du& TextureD3D9::GetSize() const
{
	return m_Dimension;
}


IDirect3DSurface9* TextureD3D9::GetTempSurface(u32 width, u32 height, D3DFORMAT format)
{
	HRESULT hr;

	core::array<IDirect3DSurface9*>::Iterator matchingFormat = s_TempSurfaces.End();
	for(auto it = s_TempSurfaces.First(); it != s_TempSurfaces.End(); ++it) {

		D3DSURFACE_DESC desc;
		(*it)->GetDesc(&desc);
		if(desc.Width <= width && desc.Height <= height && desc.Format == format) {
			if((*it)->AddRef() == 2) {
				(*it)->Release();
				if(desc.Format == format)
					matchingFormat = it;
				continue;
			}

			return *it;
		}
	}

	if(matchingFormat != s_TempSurfaces.End()) {
		IDirect3DSurface9* surface = *matchingFormat;
		s_TempSurfaces.Erase(matchingFormat);
		surface->Release();
	}

	IDirect3DDevice9* device;
	m_Texture->GetDevice(&device);
	IDirect3DSurface9* surface;
	hr = device->CreateOffscreenPlainSurface(width, height, format, D3DPOOL_SYSTEMMEM, &surface, nullptr);

	if(FAILED(hr))
		return nullptr;

	surface->AddRef();
	s_TempSurfaces.PushBack(surface);

	return surface;
}

void TextureD3D9::FreeTempSurface(IDirect3DSurface9* surface)
{
	if(surface)
		surface->Release();
}


StrongRef<Referable> TextureD3D9::Clone() const
{
	return new TextureD3D9(m_Device);
}

}
}

#endif // LUX_COMPILE_WITH_D3D9
