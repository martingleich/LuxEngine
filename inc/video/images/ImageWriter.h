#ifndef INCLUDED_IMAGEWRITER_H
#define INCLUDED_IMAGEWRITER_H
#include "video/images/Image.h"

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
	virtual bool WriteFile(io::File* File, void* data, video::ColorFormat format, math::dimension2du size, u32 pitch, u32 writerParam = 0) = 0;
	bool WriteFile(io::File* file, Image* image, u32 writerParam = 0)
	{
		if(!image)
			return false;
		if(!file)
			return false;
		void* p = image->Lock();
		bool ret = WriteFile(file, p, image->GetColorFormat(), image->GetDimension(), image->GetPitch(), writerParam);
		image->Unlock();
		return ret;
	}
};

}
}

#endif // #ifndef INCLUDED_IIMAGEWRITER