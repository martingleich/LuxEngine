#ifndef INCLUDED_IMAGELOADERD3DX_H
#define INCLUDED_IMAGELOADERD3DX_H
#include "resources/ResourceLoader.h"

#ifdef LUX_COMPILE_WITH_D3DX_IMAGE_LOADER

struct IDirect3DDevice9;

namespace lux
{
namespace video
{

class ImageLoaderD3DX : public core::ResourceLoader
{
public:
	ImageLoaderD3DX(IDirect3DDevice9* pDevice);
	~ImageLoaderD3DX();

	core::Name GetResourceType(io::File* file, core::Name requestedType);
	void LoadResource(io::File* file, core::Resource* dst);
	const String& GetName() const;

private:
	IDirect3DDevice9* m_Device;
};

}
}

#endif // LUX_COMPILE_WITH_D3DX_IMAGE_LOADER

#endif // !INCLUDED_CIMAGELOADERD3DX_H
