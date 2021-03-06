#include "ImageWriterTGA.h"
#include "io/File.h"
#include "video/ColorConverter.h"
#include "core/lxMemory.h"

#include <libtga/src/tga.h>

namespace lux
{
namespace video
{

namespace
{
template <typename T, void(*Func)(T*)>
struct destroyer
{
	T* x;
	destroyer(T* _x) : x(_x)
	{
	}
	~destroyer()
	{
		if(x) Func(x);
	}

	operator T*()
	{
		return x;
	}
};
}

bool ImageWriterTGA::CanWriteFile(const core::String& ext)
{
	return ext.IsEmpty() || ext.Equal("tga", core::EStringCompare::CaseInsensitive);
}

static uint32_t tga_proc_write(tga_struct* tga, uint32_t size, void* buffer)
{
	io::File* file = (io::File*)tga_get_io_data(tga);
	return (uint32_t)file->WriteBinaryPart(buffer, size);
}

static uint32_t tga_proc_seek(tga_struct* tga, uint32_t offset)
{
	io::File* file = (io::File*)tga_get_io_data(tga);
	try {
		file->Seek(offset, io::ESeekOrigin::Start);
	} catch(...) {
		// Don't throw a exception while using longjumps
		return 1;
	}

	return 0;
}

void ImageWriterTGA::WriteFile(io::File* file, void* data, video::ColorFormat format, math::Dimension2I size, u32 pitch, u32 writerParam)
{
	LX_CHECK_NULL_ARG(file);
	LX_CHECK_NULL_ARG(data);

	LUX_UNUSED(writerParam);

	void* writeData = data;
	u32 writePitch = pitch;
	video::ColorFormat writeFormat = format;
	core::RawMemory buffer;
	bool result;
	if(format != video::ColorFormat::A8R8G8B8 && format != video::ColorFormat::R8G8B8) {
		video::ColorFormat newFormat = format.HasAlpha() ? video::ColorFormat::A8R8G8B8 : video::ColorFormat::R8G8B8;
		buffer.SetSize(newFormat.GetBytePerPixel() * size.GetArea());
		u8* newData = (u8*)buffer;
		result = video::ColorConverter::ConvertByFormat(
			data, format,
			newData, newFormat,
			size.width, size.height,
			pitch, size.width*newFormat.GetBytePerPixel());
		if(result) {
			writeData = newData;
			writePitch = newFormat.GetBytePerPixel() * size.width;
			writeFormat = newFormat;
		} else {
			throw core::FileFormatException("Can't safe colorformat", "tga");
		}
	}

	destroyer<tga_struct, tga_destroy> tga(tga_init_write(nullptr));
	result = false;
	do {
		if(!tga)
			break;

		if(tga_set_io_data(tga, file))
			break;

		if(tga_set_io_write_proc(tga, tga_proc_write))
			break;

		if(tga_set_io_seek_proc(tga, tga_proc_seek))
			break;

		destroyer<tga_info, tga_destroy_info> info(tga_init_info(tga));
		if(!info)
			break;

		if(tga_info_set_width(info, size.width))
			break;

		if(tga_info_set_height(info, size.height))
			break;

		if(tga_info_set_pitch(info, writePitch))
			break;

		if(tga_info_set_flags(info, TGA_FLAG_COLOR_RGB | (writeFormat.HasAlpha() ? TGA_FLAG_ALPHA : 0)))
			break;

		if(tga_write_image(tga, info, writeData))
			break;

		result = true;
	} while(false);

	if(!result)
		throw core::FileFormatException("Can't write file", "tga");
}

}
}