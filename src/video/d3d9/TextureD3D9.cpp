#ifdef LUX_COMPILE_WITH_D3D9
#include "TextureD3D9.h"

#include "StrippedD3D9X.h"
#include "D3DHelper.h"
#include "D3D9Exception.h"
#include "AuxiliaryTextureD3D9.h"

namespace lux
{
namespace video
{
TextureD3D9::TextureD3D9(IDirect3DDevice9* device, const core::ResourceOrigin& origin) :
	Texture(origin),
	m_LockedLevel(0),
	m_IsLocked(false),
	m_Device(device)
{
}

TextureD3D9::~TextureD3D9()
{
}

void TextureD3D9::RegenerateMIPMaps()
{
	HRESULT hr = D3DXFilterTexture(m_Texture, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
	if(FAILED(hr))
		throw core::D3D9Exception(hr);
}

void TextureD3D9::Init(
	const math::Dimension2U& Size,
	video::ColorFormat lxFormat,
	u32 MipCount, bool isRendertarget, bool isDynamic)
{
	if(!m_Device)
		throw core::Exception("No driver available");

	if(m_Texture)
		m_Texture = nullptr;

	D3DFORMAT format = GetD3DFormat(lxFormat);
	if(format == D3DFMT_UNKNOWN)
		throw core::ColorFormatException(lxFormat);

	m_Levels = MipCount;
	if(MipCount == 0)
		m_Levels = 0;

	DWORD usage = 0;
	// Put in managed pool if there no origin loader
	D3DPOOL pool = GetOrigin().IsAvailable() ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
	if(isDynamic) {
		usage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	} if(isRendertarget) {
		usage = D3DUSAGE_RENDERTARGET;
		pool = D3DPOOL_DEFAULT;
	}

	HRESULT hr = m_Device->CreateTexture(Size.width, Size.height, m_Levels,
		usage, format,
		pool, m_Texture.Access(), nullptr);

	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	m_IsLocked = false;

	m_Texture->GetLevelDesc(0, &m_Desc);

	m_Dimension.Set(m_Desc.Width, m_Desc.Height);
	m_Format = lxFormat;
}

BaseTexture::LockedRect TextureD3D9::Lock(ELockMode Mode, u32 MipLevel)
{
	if(m_IsLocked)
		throw core::Exception("Texture is already locked");

	m_LockedLevel = MipLevel;
	if(m_LockedLevel >= m_Texture->GetLevelCount())
		m_LockedLevel = m_Texture->GetLevelCount() - 1;

	D3DLOCKED_RECT Locked;
	DWORD Flags;
	HRESULT hr;
	if(Mode == ELockMode::Overwrite && m_Desc.Usage == D3DUSAGE_DYNAMIC)
		Flags = D3DLOCK_DISCARD;
	else if(Mode == ELockMode::ReadOnly)
		Flags = D3DLOCK_READONLY;
	else
		Flags = 0;

	hr = m_Texture->LockRect(MipLevel, &Locked, nullptr, Flags);

	if(FAILED(hr)) {
		if(Mode == ELockMode::Overwrite && m_Desc.Usage == 0) {
			m_TempSurface = AuxiliaryTextureManagerD3D9::Instance()->GetSurface(m_Desc.Width, m_Desc.Height, m_Desc.Format);
			if(m_TempSurface)
				hr = m_TempSurface->LockRect(&Locked, nullptr, D3DLOCK_DISCARD);
			if(FAILED(hr)) {
				m_TempSurface = nullptr;
				throw core::D3D9Exception(hr);
			}
		} else if(Mode == ELockMode::ReadOnly && m_Desc.Usage == 0) {
			throw core::Exception("Can't lock static texture in read mode");
		} else if(Mode == ELockMode::ReadWrite && m_Desc.Usage == 0) {
			throw core::Exception("Can't lock static texture in read mode");
		} else if(Mode == ELockMode::ReadOnly && m_Desc.Usage == D3DUSAGE_RENDERTARGET) {
			m_TempSurface = AuxiliaryTextureManagerD3D9::Instance()->GetSurface(m_Desc.Width, m_Desc.Height, m_Desc.Format);
			if(m_TempSurface) {
				UnknownRefCounted<IDirect3DSurface9> surface;
				hr = m_Texture->GetSurfaceLevel(m_LockedLevel, surface.Access());
				if(SUCCEEDED(hr))
					hr = m_Device->GetRenderTargetData(surface, m_TempSurface);
				if(SUCCEEDED(hr))
					hr = m_TempSurface->LockRect(&Locked, nullptr, D3DLOCK_READONLY);
			}
			if(FAILED(hr)) {
				m_TempSurface = nullptr;
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
			UnknownRefCounted<IDirect3DSurface9> surface;
			hr = m_Texture->GetSurfaceLevel(m_LockedLevel, surface.Access());
			if(SUCCEEDED(hr))
				hr = m_Device->UpdateSurface(m_TempSurface, nullptr, surface, nullptr);
			if(FAILED(hr))
				throw core::D3D9Exception(hr);
		}

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

const math::Dimension2U& TextureD3D9::GetSize() const
{
	return m_Dimension;
}

const BaseTexture::Filter& TextureD3D9::GetFiltering() const
{
	return m_Filtering;
}
void TextureD3D9::SetFiltering(const Filter& f)
{
	m_Filtering = f;
}

void TextureD3D9::ReleaseUnmanaged()
{
	if(m_IsLocked)
		Unlock();
	if(m_Desc.Pool == D3DPOOL_DEFAULT || m_Desc.Usage == D3DUSAGE_RENDERTARGET)
		m_Texture = nullptr;
}

void TextureD3D9::RestoreUnmanaged()
{
	if(m_Desc.Usage == D3DUSAGE_RENDERTARGET) {
		Init(m_Dimension, m_Format, m_Levels, true, false);
	} else if(m_Desc.Pool == D3DPOOL_DEFAULT) {
		if(GetOrigin().IsAvailable())
			GetOrigin().Load(this);
		else
			Init(m_Dimension, m_Format, m_Levels, false, (m_Desc.Usage & D3DUSAGE_DYNAMIC) != 0);
	} else {
		// Is a managed texture
		(void)0;
	}
}

}
}

#endif // LUX_COMPILE_WITH_D3D9
