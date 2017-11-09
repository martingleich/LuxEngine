#ifndef INCLUDED_IMAGEWRITER_H
#define INCLUDED_IMAGEWRITER_H
#include "resources/ResourceWriter.h"
#include "video/images/Image.h"
#include "video/Texture.h"
#include "core/lxException.h"

namespace lux
{
namespace video
{

class ImageWriter : public core::ResourceWriter
{
public:
	bool CanWriteType(const core::String& ext, core::Name requestedType)
	{
		if(requestedType == core::ResourceType::Image || requestedType == core::ResourceType::Texture)
			return CanWriteFile(ext);
		else
			return false;
	}

	virtual bool CanWriteFile(const core::String& ext) = 0;
	
	void WriteFile(io::File* file, Image* image, u32 writerParam = 0)
	{
		LX_CHECK_NULL_ARG(file);
		LX_CHECK_NULL_ARG(image);

		video::ImageLock lock(image);
		WriteFile(file, lock.data, image->GetColorFormat(), image->GetSize(), lock.pitch, writerParam);
	}

	void WriteFile(io::File* file, Texture* texture, u32 writerParam = 0)
	{
		LX_CHECK_NULL_ARG(file);
		LX_CHECK_NULL_ARG(texture);

		video::TextureLock lock(texture, video::BaseTexture::ELockMode::ReadOnly);
		WriteFile(file, lock.data, texture->GetColorFormat(), texture->GetSize(), lock.pitch, writerParam);
	}

	void WriteResource(io::File* file, core::Resource* resource)
	{
		auto image = dynamic_cast<Image*>(resource);
		auto texture = dynamic_cast<Texture*>(resource);

		if(image)
			WriteFile(file, image);
		else if(texture)
			WriteFile(file, texture);
	}

	virtual void WriteFile(io::File* File, void* data, video::ColorFormat format, math::Dimension2U size, u32 pitch, u32 writerParam = 0) = 0;
};

}
}

#endif // #ifndef INCLUDED_IMAGEWRITER_H