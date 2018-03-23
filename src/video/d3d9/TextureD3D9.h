#ifndef INCLUDED_LUX_TEXTURE_D3D9_H
#define INCLUDED_LUX_TEXTURE_D3D9_H
#include "video/Texture.h"
#include "core/lxArray.h"

#ifdef LUX_COMPILE_WITH_D3D9

#include "platform/StrippedD3D9.h"
#include "platform/UnknownRefCounted.h"

namespace lux
{
namespace video
{

class TextureD3D9 : public Texture
{
public:
	TextureD3D9(IDirect3DDevice9* device, const core::ResourceOrigin& origin);
	~TextureD3D9();

	void Init(
		const math::Dimension2I& Size,
		ColorFormat format,
		int MipCount, bool isRendertarget, bool isDynamic);

	LockedRect Lock(ELockMode Mode, int MipLevel);
	void Unlock(bool regenMipMaps);
	void RegenerateMIPMaps();

	bool IsRendertarget() const;
	bool IsDynamic() const;

	ColorFormat GetColorFormat() const;
	void* GetRealTexture();
	int GetLevelCount() const;
	const math::Dimension2I& GetSize() const;

	const Filter& GetFiltering() const;
	void SetFiltering(const Filter& f);

	void ReleaseUnmanaged();
	void RestoreUnmanaged();

protected:
	UnknownRefCounted<IDirect3DDevice9> m_Device;
	UnknownRefCounted<IDirect3DTexture9> m_Texture;
	D3DSURFACE_DESC m_Desc;
	ColorFormat m_Format;
	Filter m_Filtering;
	int m_Levels;

	math::Dimension2I m_Dimension;

	bool m_IsLocked;
	int m_LockedLevel;
	ELockMode m_LockedMode;
	UnknownRefCounted<IDirect3DSurface9> m_TempSurface;
};

}
}

#endif // LUX_COMPILE_WITH_D3D9

#endif