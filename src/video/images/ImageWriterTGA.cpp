#include "ImageWriterTGA.h"
#include "io/File.h"
#include "video/ColorConverter.h"
#include "core/lxMemory.h"

#include "../external/libtga/src/tga.h"

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

bool ImageWriterTGA::CanWriteFile(const io::path& file)
{
	return io::GetFileExtension(file).EqualCaseInsensitive("tga");
}

static uint32_t tga_proc_write(tga_struct* tga, uint32_t size, void* buffer)
{
	io::File* file = (io::File*)tga_get_io_data(tga);
	return file->WriteBinary(buffer, size);
}

static uint32_t tga_proc_seek(tga_struct* tga, uint32_t offset)
{
	io::File* file = (io::File*)tga_get_io_data(tga);
	bool success = file->Seek(offset, io::ESeekOrigin::Start);

	return success ? 0 : 1;
}

void ImageWriterTGA::WriteFile(io::File* file, void* data, video::ColorFormat format, math::dimension2du size, u32 pitch, u32 writerParam)
{
	LX_CHECK_NULL_ARG(file);
	LX_CHECK_NULL_ARG(data);

	LUX_UNUSED(writerParam);

	void* writeData = data;
	u32 writePitch = pitch;
	video::ColorFormat writeFormat = format;
	bool freeData = false;
	if(format != video::ColorFormat::A8R8G8B8 && format != video::ColorFormat::R8G8B8) {
		video::ColorFormat newFormat = format.HasAlpha() ? video::ColorFormat::A8R8G8B8 : video::ColorFormat::R8G8B8;
		u8* newData = LUX_NEW_ARRAY(u8, newFormat.GetBytePerPixel() * size.GetArea());
		freeData = true;
		bool result = video::ColorConverter::ConvertByFormat(
			data, format,
			newData, newFormat,
			size.width, size.height,
			pitch, size.width*newFormat.GetBytePerPixel());
		if(result) {
			writeData = newData;
			writePitch = newFormat.GetBytePerPixel() * size.width;
			writeFormat = newFormat;
		}
	}

	destroyer<tga_struct, tga_destroy> tga(tga_init_write(nullptr));
	if(!tga)
		throw core::GenericException();

	if(tga_set_io_data(tga, file))
		throw core::GenericException();

	if(tga_set_io_write_proc(tga, tga_proc_write))
		throw core::GenericException();

	if(tga_set_io_seek_proc(tga, tga_proc_seek))
		throw core::GenericException();

	destroyer<tga_info, tga_destroy_info> info(tga_init_info(tga));
	if(!info)
		throw core::GenericException();

	if(tga_info_set_width(info, size.width))
		throw core::GenericException();

	if(tga_info_set_height(info, size.height))
		throw core::GenericException();

	if(tga_info_set_pitch(info, writePitch))
		throw core::GenericException();

	if(tga_info_set_flags(info, TGA_FLAG_COLOR_RGB | (writeFormat.HasAlpha() ? TGA_FLAG_ALPHA : 0)))
		throw core::GenericException();

	if(tga_write_image(tga, info, writeData))
		throw core::GenericException();

	if(freeData)
		LUX_FREE_ARRAY(writeData);
}

}
}