#ifndef INCLUDED_IMAGE_WRITER_TGA
#define INCLUDED_IMAGE_WRITER_TGA
#include "video/images/ImageWriter.h"

namespace lux
{
namespace video
{

class ImageWriterTGA : public video::ImageWriter
{
public:
	bool CanWriteFile(const io::path& file);
	bool WriteFile(io::File* file, void* data, video::ColorFormat format, math::dimension2du size, u32 pitch, u32 writerParam);
};

}
}

#endif // #ifndef INCLUDED_IMAGE_WRITER_TGA