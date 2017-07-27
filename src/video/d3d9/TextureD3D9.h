#ifndef INCLUDED_TEXTURE_D3D9_H
#define INCLUDED_TEXTURE_D3D9_H
#include "video/Texture.h"
#include "core/lxArray.h"

#ifdef LUX_COMPILE_WITH_D3D9

#include "StrippedD3D9.h"

namespace lux
{
namespace video
{

class TextureD3D9 : public Texture
{
public:
	TextureD3D9(IDirect3DDevice9* device);
	~TextureD3D9();

	void Init(
		const math::Dimension2U& Size,
		ColorFormat format,
		u32 MipCount, bool isRendertarget, bool isDynamic);

	LockedRect Lock(ELockMode Mode, u32 MipLevel);
	void Unlock();
	void RegenerateMIPMaps();

	bool IsRendertarget() const;

	ColorFormat GetColorFormat() const;
	void* GetRealTexture();
	u32 GetLevelCount() const;
	const math::Dimension2U& GetSize() const;

	const Filter& GetFiltering() const;
	void SetFiltering(const Filter& f);

private:
	static u32 s_TextureCount;
	static core::Array<IDirect3DSurface9*> s_TempSurfaces;

	IDirect3DSurface9* GetTempSurface(u32 width, u32 height, D3DFORMAT format);
	void FreeTempSurface(IDirect3DSurface9* surface);

protected:
	IDirect3DDevice9* m_Device;
	IDirect3DTexture9* m_Texture;
	D3DSURFACE_DESC m_Desc;
	ColorFormat m_Format;
	Filter m_Filtering;

	math::Dimension2U m_Dimension;

	bool m_IsLocked;
	u32 m_LockedLevel;
	ELockMode m_LockedMode;
	IDirect3DSurface9* m_TempSurface;
};

}

}


#endif // LUX_COMPILE_WITH_D3D9

#endif