#ifndef INCLUDED_LUX_IMAGE_WRITER_TGA_H
#define INCLUDED_LUX_IMAGE_WRITER_TGA_H
#include "video/images/ImageWriter.h"

namespace lux
{
namespace video
{

class ImageWriterTGA : public video::ImageWriter
{
public:
	const core::String& GetName() const
	{
		static const core::String name = "Lux TGA-Writer";
		return name;
	}

	bool CanWriteFile(const core::String& ext);
	void WriteFile(io::File* file, void* data, video::ColorFormat format, math::Dimension2I size, u32 pitch, u32 writerParam);
};

}
}

#endif // #ifndef INCLUDED_LUX_IMAGE_WRITER_TGA_H