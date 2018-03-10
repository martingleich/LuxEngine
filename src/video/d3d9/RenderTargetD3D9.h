#ifndef INCLUDED_RENDERTARGET_D3D9_H
#define INCLUDED_RENDERTARGET_D3D9_H
#include "video/RenderTarget.h"

#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "platform/StrippedD3D9.h"
#include "D3DHelper.h"
#include "platform/UnknownRefCounted.h"

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
		RenderTarget(texture)
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
	RendertargetD3D9(CubeTexture* texture, CubeTexture::EFace face) :
		RenderTarget(texture, face)
	{
		if(m_Texture) {
			lxAssert(m_Texture->IsRendertarget());
			IDirect3DCubeTexture9* d3dTexture = (IDirect3DCubeTexture9*)texture->GetRealTexture();
			if(FAILED(d3dTexture->GetCubeMapSurface(GetD3DCubeMapFace(face), 0, m_Surface.Access()))) {
				m_Texture = nullptr;
				m_Surface = nullptr;
				m_Size.Set(0, 0);
			}
		}
	}

	RendertargetD3D9(const RenderTarget& target)
	{
		auto tex = target.GetTexture();
		auto cubeTex = dynamic_cast<CubeTexture*>(tex);
		if(cubeTex && target.IsCubeTarget())
			*this = RendertargetD3D9(cubeTex, target.GetCubeFace());
		auto flatTex = dynamic_cast<Texture*>(tex);
		if(flatTex && !target.IsCubeTarget())
			*this = RendertargetD3D9(flatTex);
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