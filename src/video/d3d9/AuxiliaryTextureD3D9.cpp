#include "AuxiliaryTextureD3D9.h"
#ifdef LUX_COMPILE_WITH_D3D9

namespace lux
{
namespace video
{
static StrongRef<AuxiliaryTextureManagerD3D9> g_Instance;
AuxiliaryTextureManagerD3D9* AuxiliaryTextureManagerD3D9::Instance()
{
	return g_Instance;
}

void AuxiliaryTextureManagerD3D9::Initialize(IDirect3DDevice9* device)
{
	g_Instance = LUX_NEW(AuxiliaryTextureManagerD3D9)(device);
}

void AuxiliaryTextureManagerD3D9::Destroy()
{
	g_Instance = nullptr;
}

AuxiliaryTextureManagerD3D9::AuxiliaryTextureManagerD3D9(IDirect3DDevice9* device) :
	m_Device(device)
{
}

AuxiliaryTextureManagerD3D9::~AuxiliaryTextureManagerD3D9()
{
}

UnknownRefCounted<IDirect3DSurface9> AuxiliaryTextureManagerD3D9::GetSurface(DWORD width, DWORD height, D3DFORMAT format, bool exactSize)
{
	HRESULT hr;

	for(auto it = m_Surfaces.First(); it != m_Surfaces.End(); ++it) {
		bool matchSize;
		if(exactSize)
			matchSize = it->desc.Width == width && it->desc.Height == height;
		else
			matchSize = it->desc.Width >= width && it->desc.Height >= height;
		if(matchSize && it->desc.Format == format) {
			ULONG refCount = it->surface->AddRef() - 1;
			it->surface->Release();
			if(refCount == 1)
				return it->surface;
		}
	}

	UnknownRefCounted<IDirect3DSurface9> surface;
	hr = m_Device->CreateOffscreenPlainSurface(width, height, format, D3DPOOL_SYSTEMMEM, surface.Access(), nullptr);
	if(FAILED(hr)) {
		// Sometime it's possible to create a texture and use its surface.
		UnknownRefCounted<IDirect3DTexture9> texture;
		hr = m_Device->CreateTexture(width, height, 1, 0, format, D3DPOOL_SYSTEMMEM, texture.Access(), nullptr);
		if(SUCCEEDED(hr))
			hr = texture->GetSurfaceLevel(0, surface.Access());
	}
	if(FAILED(hr))
		surface = nullptr;
	else {
		if(m_Surfaces.Size() > MAX_TEXTURES) {
			for(auto it = m_Surfaces.First(); it != m_Surfaces.End();) {
				if(it->surface->Release() == 0) {
					it = m_Surfaces.Erase(it);
				} else {
					it->surface->AddRef();
					++it;
				}
			}
		}

		m_Surfaces.PushBack(surface);
	}

	return surface;
}

void AuxiliaryTextureManagerD3D9::ReleaseUnmanaged()
{
	m_Surfaces.Clear();
}

void AuxiliaryTextureManagerD3D9::RestoreUnmanaged()
{
}

} // namespace video
} // namespace lux

#endif
