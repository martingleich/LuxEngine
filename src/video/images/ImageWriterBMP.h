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
	const string& GetName() const
	{
		static const string name = "Lux BMP-Writer";
		return name;
	}

	bool CanWriteFile(const string& ext);
	void WriteFile(io::File* File, void* Data, video::ColorFormat Format, math::dimension2du Size, u32 Pitch, u32 WriterParam = 0);
};

}
}
#endif // !INCLUDED_IMAGEWRITERBMP