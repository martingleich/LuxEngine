#ifndef INCLUDED_AUXILIARY_TEXTURE_H
#define INCLUDED_AUXILIARY_TEXTURE_H

#ifdef LUX_COMPILE_WITH_D3D9
#include "core/ReferenceCounted.h"
#include "platform/UnknownRefCounted.h"
#include "core/lxArray.h"

#include "platform/StrippedD3D9.h"

namespace lux
{
namespace video
{

class AuxiliaryTextureManagerD3D9 : public ReferenceCounted
{
public:
	static AuxiliaryTextureManagerD3D9* Instance();
	static void Initialize(IDirect3DDevice9* device);
	static void Destroy();

	AuxiliaryTextureManagerD3D9(IDirect3DDevice9* device);
	~AuxiliaryTextureManagerD3D9();

	UnknownRefCounted<IDirect3DSurface9> GetSurface(DWORD width, DWORD height, D3DFORMAT format, bool exactSize = true);

	void ReleaseUnmanaged();
	void RestoreUnmanaged();

private:
	struct Entry
	{
		UnknownRefCounted<IDirect3DSurface9> surface;
		D3DSURFACE_DESC desc;

		Entry(UnknownRefCounted<IDirect3DSurface9> _surface) :
			surface(_surface)
		{
			lxAssert(surface);
			surface->GetDesc(&desc);
		}
	};

	core::Array<Entry> m_Surfaces;
	UnknownRefCounted<IDirect3DDevice9> m_Device;
	static const u32 MAX_TEXTURES = 5;
};

}
}

#endif

#endif // #ifndef INCLUDED_AUXILIARY_TEXTURE_H