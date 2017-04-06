#ifndef INCLUDED_TEXTURED3D9_H
#define INCLUDED_TEXTURED3D9_H
#include "video/Texture.h"
#include "core/lxArray.h"
#include "StrippedD3D9.h"

namespace lux
{
namespace video
{

class TextureD3D9 : public Texture
{
protected:
	IDirect3DDevice9* m_Device;
	IDirect3DTexture9* m_Texture;
	D3DSURFACE_DESC m_Desc;
	ColorFormat m_Format;

	math::dimension2du m_Dimension;

	bool m_IsLocked;
	u32 m_LockedLevel;
	ETextureLockMode m_LockedMode;
	IDirect3DSurface9* m_TempSurface;

private:
	static u32 s_TextureCount;
	static core::array<IDirect3DSurface9*> s_TempSurfaces;

	IDirect3DSurface9* GetTempSurface(u32 width, u32 height, D3DFORMAT format);
	void FreeTempSurface(IDirect3DSurface9* surface);

public:
	TextureD3D9(IDirect3DDevice9* device);
	~TextureD3D9();

	bool Init(
		const math::dimension2du& Size,
		ColorFormat format,
		u32 MipCount, bool isRendertarget, bool isDynamic);

	void* Lock(ETextureLockMode Mode, SLockedRect* locked, u32 MipLevel);
	void Unlock();
	void RegenerateMIPMaps();

	bool IsRendertarget() const;

	ColorFormat GetColorFormat() const;
	void* GetRealTexture();
	u32 GetLevelCount() const;
	const math::dimension2du& GetDimension() const;

	StrongRef<Referable> Clone() const;
};

}    // namespace video
}    // namespace lux

#endif