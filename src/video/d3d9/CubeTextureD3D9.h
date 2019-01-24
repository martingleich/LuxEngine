#ifndef INCLUDED_LUX_CUBETEXTURE_D3D9_H
#define INCLUDED_LUX_CUBETEXTURE_D3D9_H
#include "video/CubeTexture.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "platform/StrippedD3D9.h"
#include "AuxiliaryTextureD3D9.h"

namespace lux
{
namespace video
{

class CubeTextureD3D9 : public CubeTexture
{
public:
	CubeTextureD3D9(IDirect3DDevice9* d3dDevice);
	~CubeTextureD3D9();

	void Init(int size, ColorFormat lxFormat, bool isRendertarget, bool isDynamic) override;

	LockedRect Lock(ELockMode mode, EFace face) override;
	void Unlock() override;

	ColorFormat GetColorFormat() const override { return m_Format; }
	void* GetRealTexture() override { return m_Texture; }
	int GetSize() const override { return m_Size; }
	bool IsRendertarget() const override { return m_Usage == D3DUSAGE_RENDERTARGET; }
	bool IsDynamic() const override { return m_Usage == D3DUSAGE_DYNAMIC; }

	const Filter& GetFiltering() const { return m_Filtering; }
	void SetFiltering(const Filter& f) { m_Filtering = f; }

	void ReleaseUnmanaged();
	void RestoreUnmanaged();

private:
	UnknownRefCounted<IDirect3DDevice9> m_D3DDevice;
	UnknownRefCounted<IDirect3DCubeTexture9> m_Texture;

	bool m_IsLocked;
	D3DCUBEMAP_FACES m_LockedFace;
	UnknownRefCounted<IDirect3DSurface9> m_TempSurface;

	DWORD m_Usage;
	D3DPOOL m_Pool;
	D3DFORMAT m_D3DFormat;
	ColorFormat m_Format;
	Filter m_Filtering;
	int m_Size;
};

}

}

#endif // LUX_COMPILE_WITH_D3D9

#endif