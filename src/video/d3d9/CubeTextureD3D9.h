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
public:
	CubeTextureD3D9(IDirect3DDevice9* d3dDevice);
	virtual ~CubeTextureD3D9();

	void Init(u32 Size, ColorFormat lxFormat, bool isDynamic);

	void RegenerateMIPMaps();

	LockedRect Lock(ELockMode Mode, EFace Face, u32 MipLevel = 0);
	void Unlock();

	ColorFormat GetColorFormat() const;
	void* GetRealTexture();
	u32 GetLevelCount() const;
	const math::Dimension2U& GetSize() const;

	const Filter& GetFiltering() const;
	void SetFiltering(const Filter& f);

private:
	IDirect3DCubeTexture9* m_Texture;
	u32 m_LockedLevel;
	D3DCUBEMAP_FACES m_LockedFace;
	D3DSURFACE_DESC m_Desc;
	ColorFormat m_Format;
	Filter m_Filtering;

	math::Dimension2U m_Dimension;

	IDirect3DDevice9* m_D3DDevice;
};

}

}

#endif // LUX_COMPILE_WITH_D3D9

#endif