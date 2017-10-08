#ifdef LUX_COMPILE_WITH_D3D9
#include "CubeTextureD3D9.h"
#include "StrippedD3D9X.h"
#include "D3D9Exception.h"
#include "D3DHelper.h"

namespace lux
{
namespace video
{

CubeTextureD3D9::CubeTextureD3D9(IDirect3DDevice9* dev, const core::ResourceOrigin& origin) :
	CubeTexture(origin),
	m_D3DDevice(dev),
	m_LockedLevel(0xFFFFFFFF)
{
}

CubeTextureD3D9::~CubeTextureD3D9()
{
}

void CubeTextureD3D9::Init(u32 size, ColorFormat lxFormat, bool isRendertarget, bool isDynamic)
{
	if(!m_D3DDevice)
		throw core::Exception("No driver available");

	if(m_Texture)
		m_Texture = nullptr;

	D3DFORMAT format = GetD3DFormat(lxFormat);
	if(format == D3DFMT_UNKNOWN)
		throw core::ColorFormatException(lxFormat);

	DWORD usage = 0;
	D3DPOOL pool = GetOrigin().loader ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
	if(isDynamic) {
		usage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	} if(isRendertarget) {
		usage = D3DUSAGE_RENDERTARGET;
		pool = D3DPOOL_DEFAULT;
	}
	HRESULT hr = m_D3DDevice->CreateCubeTexture(size, 1, usage, format, pool, m_Texture.Access(), nullptr);

	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	m_LockedLevel = 0xFFFFFFFF;

	m_Texture->GetLevelDesc(0, &m_Desc);

	m_Dimension.Set(m_Desc.Width, m_Desc.Height);

	m_Format = lxFormat;
}

void CubeTextureD3D9::RegenerateMIPMaps()
{
	HRESULT hr = D3DXFilterTexture(m_Texture, nullptr, 0, 0);
	if(FAILED(hr))
		throw core::D3D9Exception(hr);
}

BaseTexture::LockedRect CubeTextureD3D9::Lock(ELockMode mode, EFace face, u32 mipLevel)
{
	if(m_LockedLevel != 0xFFFFFFFF)
		throw core::Exception("Texture is already locked");

	m_LockedLevel = mipLevel;
	m_LockedFace = GetD3DCubeMapFace(face);

	D3DLOCKED_RECT d3dlocked;
	DWORD flags = 0;
	if(mode == ELockMode::Overwrite && m_Desc.Usage == D3DUSAGE_DYNAMIC)
		flags = D3DLOCK_DISCARD;
	else if(mode == ELockMode::ReadOnly)
		flags = D3DLOCK_READONLY;

	HRESULT hr = m_Texture->LockRect(m_LockedFace, mipLevel, &d3dlocked, nullptr, flags);
	if(FAILED(hr)) {
		if(mode == ELockMode::Overwrite && m_Desc.Usage == 0) {
			m_TempSurface = AuxiliaryTextureManagerD3D9::Instance()->GetSurface(m_Desc.Width, m_Desc.Height, m_Desc.Format);
			if(m_TempSurface)
				hr = m_TempSurface->LockRect(&d3dlocked, nullptr, D3DLOCK_DISCARD);
			if(FAILED(hr)) {
				m_TempSurface = nullptr;
				throw core::D3D9Exception(hr);
			}
		} else if(mode == ELockMode::ReadOnly && m_Desc.Usage == 0) {
			throw core::Exception("Can't lock static texture in read mode");
		} else if(mode == ELockMode::ReadWrite && m_Desc.Usage == 0) {
			throw core::Exception("Can't lock static texture in read mode");
		}
	}
	LockedRect locked;
	locked.bits = (u8*)d3dlocked.pBits;
	locked.pitch = d3dlocked.Pitch;

	return locked;
}

void CubeTextureD3D9::Unlock(bool regenMipMaps)
{
	if(m_LockedLevel == 0xFFFFFFFF)
		return;

	HRESULT hr;
	if(m_TempSurface) {
		m_TempSurface->UnlockRect();
		if(m_Desc.Usage == 0) {
			UnknownRefCounted<IDirect3DSurface9> surface;
			hr = m_Texture->GetCubeMapSurface(m_LockedFace, m_LockedLevel, surface.Access());
			if(SUCCEEDED(hr))
				hr = m_D3DDevice->UpdateSurface(m_TempSurface, nullptr, surface, nullptr);
			if(FAILED(hr))
				throw core::D3D9Exception(hr);
		}

		m_TempSurface = nullptr;
	} else {
		m_Texture->UnlockRect(m_LockedFace, m_LockedLevel);
	}

	m_LockedLevel = 0xFFFFFFFF;
	if(regenMipMaps)
		RegenerateMIPMaps();
}

const math::Dimension2U& CubeTextureD3D9::GetSize() const
{
	return m_Dimension;
}

u32 CubeTextureD3D9::GetLevelCount() const
{
	return m_Texture->GetLevelCount();
}

const BaseTexture::Filter& CubeTextureD3D9::GetFiltering() const
{
	return m_Filtering;
}

void CubeTextureD3D9::SetFiltering(const Filter& f)
{
	m_Filtering = f;
}

void* CubeTextureD3D9::GetRealTexture()
{
	return (void*)(m_Texture);
}

ColorFormat CubeTextureD3D9::GetColorFormat() const
{
	return m_Format;
}

bool CubeTextureD3D9::IsRendertarget() const
{
	return m_Desc.Usage == D3DUSAGE_RENDERTARGET;
}

bool CubeTextureD3D9::IsDynamic() const
{
	return m_Desc.Usage == D3DUSAGE_DYNAMIC;
}

void CubeTextureD3D9::ReleaseUnmanaged()
{
	if(m_LockedLevel != 0xFFFFFFFF)
		Unlock(false);
	if(m_Desc.Pool == D3DPOOL_DEFAULT || m_Desc.Usage == D3DUSAGE_RENDERTARGET)
		m_Texture = nullptr;
}

void CubeTextureD3D9::RestoreUnmanaged()
{
	if(IsRendertarget()) {
		Init(m_Desc.Width, m_Format, true, false);
	} else if(m_Desc.Pool == D3DPOOL_DEFAULT) {
		if(GetOrigin().IsAvailable())
			GetOrigin().Load(this);
		else
			Init(m_Desc.Width, m_Format, false, IsDynamic());
	} else {
		// Is a managed texture
		(void)0;
	}
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9