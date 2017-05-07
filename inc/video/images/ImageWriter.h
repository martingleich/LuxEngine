#ifndef INCLUDED_IMAGEWRITER_H
#define INCLUDED_IMAGEWRITER_H
#include "video/images/Image.h"
#include "core/lxException.h"

namespace lux
{
namespace io
{
class File;
}

namespace video
{

class ImageWriter : public ReferenceCounted
{
public:
	virtual bool CanWriteFile(const io::path& file) = 0;
	virtual void WriteFile(io::File* File, void* data, video::ColorFormat format, math::dimension2du size, u32 pitch, u32 writerParam = 0) = 0;
	void WriteFile(io::File* file, Image* image, u32 writerParam = 0)
	{
		LX_CHECK_NULL_ARG(file);
		LX_CHECK_NULL_ARG(image);

		video::ImageLock lock(image);
		WriteFile(file, lock.data, image->GetColorFormat(), image->GetSize(), lock.pitch, writerParam);
	}
};

}
}

#endif // #ifndef INCLUDED_IMAGEWRITER_H