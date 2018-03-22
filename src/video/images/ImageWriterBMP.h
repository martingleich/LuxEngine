#ifndef INCLUDED_IMAGEWRITERBMP_H
#define INCLUDED_IMAGEWRITERBMP_h
#include "video/images/ImageWriter.h"

namespace lux
{
namespace video
{

class ImageWriterBMP : public ImageWriter
{
public:
	const core::String& GetName() const
	{
		static const core::String name = "Lux BMP-Writer";
		return name;
	}

	bool CanWriteFile(const core::String& ext);
	void WriteFile(io::File* File, void* Data, video::ColorFormat Format, math::Dimension2I Size, u32 Pitch, u32 WriterParam = 0);
};

}
}
#endif // !INCLUDED_IMAGEWRITERBMP