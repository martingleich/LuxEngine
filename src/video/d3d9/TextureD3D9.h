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
	TextureD3D9(IDirect3DDevice9* device);
	~TextureD3D9();

	void Init(
		const math::Dimension2I& size,
		ColorFormat format,
		int mipCount, bool isRendertarget, bool isDynamic);

	int GetMipMapCount() override { return m_Levels; }
	LockedRect Lock(ELockMode Mode, int mipLevel) override;
	void Unlock(bool regenMipMaps, int mipLevel) override;

	bool IsRendertarget() const override { return (m_Usage == D3DUSAGE_RENDERTARGET); }
	bool IsDynamic() const override { return (m_Usage == D3DUSAGE_DYNAMIC); }
	ColorFormat GetColorFormat() const override { return m_Format; }
	void* GetRealTexture() override { return (void*)(m_Texture); }
	const math::Dimension2I& GetSize() const override { return m_Size; }

	const BaseTexture::Filter& GetFiltering() const override { return m_Filtering; }
	void SetFiltering(const Filter& f) override { m_Filtering = f; }

	void ReleaseUnmanaged();
	void RestoreUnmanaged();

protected:
	UnknownRefCounted<IDirect3DDevice9> m_Device;
	UnknownRefCounted<IDirect3DTexture9> m_Texture;

	DWORD m_Usage;
	D3DPOOL m_Pool;
	D3DFORMAT m_D3DFormat;
	ColorFormat m_Format;
	Filter m_Filtering;
	int m_Levels;
	math::Dimension2I m_Size;

	u32 m_LockedLevels;
	ELockMode m_LockedMode;
	UnknownRefCounted<IDirect3DSurface9> m_TempSurface;
};

}
}

#endif // LUX_COMPILE_WITH_D3D9

#endif