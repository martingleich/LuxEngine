#include "CubeTextureD3D9.h"
#include "StrippedD3D9X.h"
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

bool CubeTextureD3D9::Init(u32 Size, ColorFormat lxFormat, bool isDynamic)
{
	if(!m_D3DDevice)
		return false;

	if(m_Texture) {
		m_Texture->Release();
		m_Texture = nullptr;
	}

	D3DFORMAT format = GetD3DFormat(lxFormat, lxFormat.HasAlpha());
	DWORD usage = 0;
	D3DPOOL pool = D3DPOOL_MANAGED;
	if(isDynamic) {
		usage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
	}
	HRESULT hr = m_D3DDevice->CreateCubeTexture(Size, 1, usage, format, pool, &m_Texture, nullptr);

	if(FAILED(hr))
		return false;

	m_LockedLevel = 0xFFFFFFFF;

	m_Texture->GetLevelDesc(0, &m_Desc);

	m_Dimension.Set(m_Desc.Width, m_Desc.Height);

	m_Format = lxFormat;

	return true;
}

void CubeTextureD3D9::RegenerateMIPMaps()
{
	D3DXFilterCubeTexture(m_Texture, nullptr, 0, 0);
}

void* CubeTextureD3D9::Lock(ETextureLockMode Mode, EFace Face, SLockedRect* locked, u32 MipLevel)
{
	if(m_LockedLevel != 0xFFFFFFFF)
		return nullptr;

	static const D3DCUBEMAP_FACES Conv[6] = {D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X,
		D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y,
		D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z};
	m_LockedLevel = MipLevel;
	m_LockedFace = Conv[(u32)Face];

	D3DLOCKED_RECT Locked;
	DWORD Flags = 0;
	if(Mode == ETLM_OVERWRITE && m_Desc.Usage == D3DUSAGE_DYNAMIC)
		Flags = D3DLOCK_DISCARD;
	else if(Mode == ETLM_READ_ONLY)
		Flags = D3DLOCK_READONLY;

	HRESULT h = m_Texture->LockRect(m_LockedFace, MipLevel, &Locked, nullptr, Flags);
	if(locked) {
		locked->bits = Locked.pBits;
		locked->pitch = Locked.Pitch;
	}

	if(FAILED(h))
		return nullptr;
	else
		return Locked.pBits;
}

void CubeTextureD3D9::Unlock()
{
	if(m_LockedLevel != 0xFFFFFFFF)
		m_Texture->UnlockRect(m_LockedFace, m_LockedLevel);

	m_LockedLevel = 0xFFFFFFFF;
}

const math::dimension2du& CubeTextureD3D9::GetDimension() const
{
	return m_Dimension;
}

u32 CubeTextureD3D9::GetLevelCount() const
{
	return m_Texture->GetLevelCount();
}

void* CubeTextureD3D9::GetRealTexture()
{
	return (void*)(m_Texture);
}

ColorFormat CubeTextureD3D9::GetColorFormat() const
{
	return m_Format;
}

StrongRef<Referable> CubeTextureD3D9::Clone() const
{
	return new CubeTextureD3D9(m_D3DDevice);
}

}    // namespace video
}    // namespace lux