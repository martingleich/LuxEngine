#ifndef INCLUDED_IMAGELOADERD3DX_H
#define INCLUDED_IMAGELOADERD3DX_H
#include "resources/ResourceLoader.h"

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
	bool LoadResource(io::File* file, core::Resource* dst);
	const string& GetName() const;

private:
	IDirect3DDevice9* m_Device;
};

}
}

#endif // !INCLUDED_CIMAGELOADERD3DX_H
