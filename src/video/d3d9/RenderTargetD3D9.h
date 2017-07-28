#ifndef INCLUDED_RENDERTARGET_D3D9_H
#define INCLUDED_RENDERTARGET_D3D9_H
#include "video/RenderTarget.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "StrippedD3D9.h"

#include "UnknownRefCounted.h"

namespace lux
{
namespace video
{

class RendertargetD3D9 : public RenderTarget
{
public:
	RendertargetD3D9()
	{
	}

	RendertargetD3D9(Texture* texture) :
		RenderTarget(texture),
		m_Surface(nullptr)
	{
		if(m_Texture) {
			lxAssert(m_Texture->IsRendertarget());
			IDirect3DTexture9* d3dTexture = (IDirect3DTexture9*)texture->GetRealTexture();
			if(FAILED(d3dTexture->GetSurfaceLevel(0, m_Surface.Access()))) {
				m_Texture = nullptr;
				m_Surface = nullptr;
				m_Size.Set(0, 0);
			}
		}
	}

	RendertargetD3D9(const RenderTarget& target) :
		RendertargetD3D9(target.GetTexture())
	{
	}

	RendertargetD3D9(UnknownRefCounted<IDirect3DSurface9> surface) :
		m_Surface(surface)
	{
		if(m_Surface) {
			D3DSURFACE_DESC desc;
			if(SUCCEEDED(m_Surface->GetDesc(&desc))) {
				lxAssert(desc.Usage == D3DUSAGE_RENDERTARGET);
				m_Size.width = desc.Width;
				m_Size.height = desc.Height;
			}
		}
	}

	IDirect3DSurface9* GetSurface()
	{
		return m_Surface;
	}

	bool operator==(const RendertargetD3D9& other)
	{
		return m_Surface == other.m_Surface;
	}

private:
	UnknownRefCounted<IDirect3DSurface9> m_Surface;
};

}
}

#endif


#endif // #ifndef INCLUDED_RENDERTARGET_D3D9_H