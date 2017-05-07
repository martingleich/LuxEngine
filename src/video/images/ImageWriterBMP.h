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
	bool CanWriteFile(const io::path& file);
	void WriteFile(io::File* File, void* Data, video::ColorFormat Format, math::dimension2du Size, u32 Pitch, u32 WriterParam = 0);
};

}
}
#endif // !INCLUDED_IMAGEWRITERBMP