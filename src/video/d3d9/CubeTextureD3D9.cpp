#ifdef LUX_COMPILE_WITH_D3D9
#include "CubeTextureD3D9.h"
#include "StrippedD3D9X.h"
#include "D3D9Exception.h"
#include "D3DHelper.h"

namespace lux
{
namespace video
{

CubeTextureD3D9::CubeTextureD3D9(IDirect3DDevice9* dev) :
	m_Texture(nullptr),
	m_LockedLevel(0xFFFFFFFF),
	m_D3DDevice(dev)
{
}

CubeTextureD3D9::~CubeTextureD3D9()
{
	if(m_Texture)
		m_Texture->Release();
}

void CubeTextureD3D9::Init(u32 Size, ColorFormat lxFormat, bool isDynamic)
{
	if(!m_D3DDevice)
		throw core::Exception("No driver available");

	if(m_Texture) {
		m_Texture->Release();
		m_Texture = nullptr;
	}

	D3DFORMAT format = GetD3DFormat(lxFormat, lxFormat.HasAlpha());
	if(format == D3DFMT_UNKNOWN)
		throw core::ColorFormatException(lxFormat);

	DWORD usage = 0;
	D3DPOOL pool = D3DPOOL_MANAGED;
	if(isDynamic) {
		usage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	}
	HRESULT hr = m_D3DDevice->CreateCubeTexture(Size, 1, usage, format, pool, &m_Texture, nullptr);

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

BaseTexture::LockedRect CubeTextureD3D9::Lock(ELockMode Mode, EFace Face, u32 MipLevel)
{
	if(m_LockedLevel != 0xFFFFFFFF)
		throw core::Exception("Texture is already locked");

	static const D3DCUBEMAP_FACES Conv[6] = {D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X,
		D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y,
		D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z};
	m_LockedLevel = MipLevel;
	m_LockedFace = Conv[(u32)Face];

	D3DLOCKED_RECT Locked;
	DWORD Flags = 0;
	if(Mode == ELockMode::Overwrite && m_Desc.Usage == D3DUSAGE_DYNAMIC)
		Flags = D3DLOCK_DISCARD;
	else if(Mode == ELockMode::ReadOnly)
		Flags = D3DLOCK_READONLY;

	HRESULT hr = m_Texture->LockRect(m_LockedFace, MipLevel, &Locked, nullptr, Flags);
	if(FAILED(hr))
		throw core::D3D9Exception(hr);

	LockedRect locked;
	locked.bits = (u8*)Locked.pBits;
	locked.pitch = Locked.Pitch;

	return locked;
}

void CubeTextureD3D9::Unlock()
{
	if(m_LockedLevel != 0xFFFFFFFF)
		m_Texture->UnlockRect(m_LockedFace, m_LockedLevel);

	m_LockedLevel = 0xFFFFFFFF;
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

}
}

#endif // LUX_COMPILE_WITH_D3D9