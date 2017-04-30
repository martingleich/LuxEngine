#ifndef INCLUDED_CUBETEXTURE_D3D9_H
#define INCLUDED_CUBETEXTURE_D3D9_H

#include "video/CubeTexture.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "StrippedD3D9.h"

namespace lux
{
namespace video
{

class CubeTextureD3D9 : public CubeTexture
{
private:
	IDirect3DCubeTexture9* m_Texture;
	u32 m_LockedLevel;
	D3DCUBEMAP_FACES m_LockedFace;
	D3DSURFACE_DESC m_Desc;
	ColorFormat m_Format;

	math::dimension2du m_Dimension;

	IDirect3DDevice9* m_D3DDevice;

public:
	CubeTextureD3D9(IDirect3DDevice9* d3dDevice);
	virtual ~CubeTextureD3D9();

	bool Init(u32 Size, ColorFormat lxFormat, bool isDynamic);

	void RegenerateMIPMaps();

	void* Lock(ETextureLockMode Mode, EFace Face, SLockedRect* locked = nullptr, u32 MipLevel = 0);
	void Unlock();

	ColorFormat GetColorFormat() const;
	void* GetRealTexture();
	u32 GetLevelCount() const;
	const math::dimension2du& GetSize() const;

	StrongRef<Referable> Clone() const;
};

}

}

#endif // LUX_COMPILE_WITH_D3D9

#endif