#include "TextureD3D9.h"
#include "StrippedD3D9X.h"
#include "D3DHelper.h"

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
	D3DXFilterTexture(m_Texture, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
}

bool TextureD3D9::Init(
	const math::dimension2du& Size,
	video::ColorFormat lxFormat,
	u32 MipCount, bool isRendertarget, bool isDynamic)
{
	if(!m_Device)
		return false;

	if(m_Texture)
		m_Texture->Release();

	D3DFORMAT format = GetD3DFormat(lxFormat, lxFormat.HasAlpha());

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

	m_Device->CreateTexture(Size.width, Size.height, Levels,
		usage, format,
		D3DPOOL_DEFAULT, &m_Texture, nullptr);

	if(m_Texture == nullptr)
		return false;

	m_IsLocked = false;

	m_Texture->GetLevelDesc(0, &m_Desc);

	m_Dimension.Set(m_Desc.Width, m_Desc.Height);
	m_Format = lxFormat;

	return true;
}

void* TextureD3D9::Lock(ETextureLockMode Mode, SLockedRect* locked, u32 MipLevel)
{
	if(m_IsLocked)
		return nullptr;

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
				hr= m_TempSurface->LockRect(&Locked, nullptr, D3DLOCK_DISCARD);
			if(FAILED(hr)) {
				FreeTempSurface(m_TempSurface);
				return nullptr;
			}
		} else if(Mode == ETLM_READ_ONLY && m_Desc.Usage == 0) {
			return nullptr;
		} else if(Mode == ETLM_READ_WRITE && m_Desc.Usage == 0) {
			return nullptr;
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
				return nullptr;
			}
		}
	}

	if(locked) {
		locked->bits = Locked.pBits;
		locked->pitch = Locked.Pitch;
		m_IsLocked = true;
	}

	return Locked.pBits;
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
		}

		FreeTempSurface(m_TempSurface);
		m_TempSurface = nullptr;
	} else {
		hr = m_Texture->UnlockRect(m_LockedLevel);
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

const math::dimension2du& TextureD3D9::GetDimension() const
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
	s_TempSurfaces.Push_Back(surface);

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

}    // namespace video
}    // namespace lux