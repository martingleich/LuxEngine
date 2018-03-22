#ifndef INCLUDED_CUBETEXTURE_D3D9_H
#define INCLUDED_CUBETEXTURE_D3D9_H
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
	CubeTextureD3D9(IDirect3DDevice9* d3dDevice, const core::ResourceOrigin& origin);
	virtual ~CubeTextureD3D9();

	void Init(int size, ColorFormat lxFormat, bool isRendertarget, bool isDynamic);

	void RegenerateMIPMaps();

	LockedRect Lock(ELockMode mode, EFace face, int mipLevel = 0);
	void Unlock(bool regenMipMaps);

	ColorFormat GetColorFormat() const;
	void* GetRealTexture();
	int GetLevelCount() const;
	const math::Dimension2I& GetSize() const;
	bool IsRendertarget() const;
	bool IsDynamic() const;

	const Filter& GetFiltering() const;
	void SetFiltering(const Filter& f);

	void ReleaseUnmanaged();
	void RestoreUnmanaged();

private:
	UnknownRefCounted<IDirect3DDevice9> m_D3DDevice;
	UnknownRefCounted<IDirect3DCubeTexture9> m_Texture;

	int m_LockedLevel;
	D3DCUBEMAP_FACES m_LockedFace;
	UnknownRefCounted<IDirect3DSurface9> m_TempSurface;

	D3DSURFACE_DESC m_Desc;
	ColorFormat m_Format;
	Filter m_Filtering;

	math::Dimension2I m_Dimension;
};

}

}

#endif // LUX_COMPILE_WITH_D3D9

#endif