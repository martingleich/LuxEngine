#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "CubeTextureD3D9.h"
#include "platform/StrippedD3D9X.h"
#include "platform/D3D9Exception.h"
#include "D3DHelper.h"

namespace lux
{
namespace video
{

CubeTextureD3D9::CubeTextureD3D9(IDirect3DDevice9* dev) :
	m_D3DDevice(dev)
{
	LX_CHECK_NULL_ARG(dev);
}

CubeTextureD3D9::~CubeTextureD3D9()
{
}

void CubeTextureD3D9::Init(int size, ColorFormat lxFormat, bool isRendertarget, bool isDynamic)
{
	if(m_Texture)
		m_Texture = nullptr;

	D3DFORMAT format = GetD3DFormat(lxFormat);
	if(format == D3DFMT_UNKNOWN)
		throw core::UnsupportedColorFormatException(lxFormat);

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

	HRESULT hr = m_D3DDevice->CreateCubeTexture((UINT)size, 1, usage, format, pool, m_Texture.Access(), nullptr);
	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	m_IsLocked = false;

	m_Usage = usage;
	m_Pool = pool;
	m_D3DFormat = format;
	m_Size = size;
	m_Format = lxFormat;
}

BaseTexture::LockedRect CubeTextureD3D9::Lock(ELockMode mode, EFace face)
{
	if(m_IsLocked)
		throw core::InvalidOperationException("Texture is already locked");

	m_LockedFace = GetD3DCubeMapFace(face);

	D3DLOCKED_RECT d3dlocked;
	DWORD flags = 0;
	if(mode == ELockMode::Overwrite && m_Usage == D3DUSAGE_DYNAMIC)
		flags = D3DLOCK_DISCARD;
	else if(mode == ELockMode::ReadOnly)
		flags = D3DLOCK_READONLY;

	HRESULT hr = m_Texture->LockRect(m_LockedFace, (UINT)0, &d3dlocked, nullptr, flags);
	if(FAILED(hr)) {
		if(mode == ELockMode::Overwrite && m_Usage == 0) {
			m_TempSurface = AuxiliaryTextureManagerD3D9::Instance()->GetSurface(DWORD(m_Size), DWORD(m_Size), m_D3DFormat);
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
		}
	}
	LockedRect locked;
	locked.bits = (u8*)d3dlocked.pBits;
	locked.pitch = d3dlocked.Pitch;

	m_IsLocked = true;
	return locked;
}

void CubeTextureD3D9::Unlock()
{
	if(m_IsLocked == false)
		return;

	HRESULT hr;
	if(m_TempSurface) {
		m_TempSurface->UnlockRect();
		if(m_Usage == 0) {
			UnknownRefCounted<IDirect3DSurface9> surface;
			hr = m_Texture->GetCubeMapSurface(m_LockedFace, 0, surface.Access());
			if(SUCCEEDED(hr))
				hr = m_D3DDevice->UpdateSurface(m_TempSurface, nullptr, surface, nullptr);
			if(FAILED(hr))
				throw core::D3D9Exception(hr);
		}

		m_TempSurface = nullptr;
	} else {
		m_Texture->UnlockRect(m_LockedFace, 0);
	}

	m_IsLocked = false;
}

void CubeTextureD3D9::ReleaseUnmanaged()
{
	if(m_Pool == D3DPOOL_MANAGED)
		return;
	m_Texture = nullptr;
}

void CubeTextureD3D9::RestoreUnmanaged()
{
	if(m_Pool == D3DPOOL_MANAGED)
		return;

	Init(m_Size, m_Format, IsRendertarget(), IsDynamic());
}

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9