#include "ImageLoaderPNG.h"
#include "video/images/Image.h"
#include "io/File.h"
#include "../external/libpng/src/png.h"

namespace lux
{
namespace video
{


void png_read_function(png_structp png_ptr, png_bytep data, png_size_t count)
{
	io::File* f = (io::File*)png_get_io_ptr(png_ptr);
	if(!f)
		png_error(png_ptr, "No read file defined");

	if(count != (png_size_t)f->ReadBinary((u32)count, data))
		png_error(png_ptr, "Unexpected end of file");
}

void png_error_handler(png_structp png_ptr, png_const_charp msg)
{
	LUX_UNUSED(msg);
	ImageLoaderPNG::MainProgInfo* info = (ImageLoaderPNG::MainProgInfo*)png_get_error_ptr(png_ptr);
	longjmp(info->jmpbuf, 1);
}

void png_warning_handler(png_structp png_ptr, png_const_charp msg)
{
	LUX_UNUSED(png_ptr);
	LUX_UNUSED(msg);
}

const string& ImageLoaderPNG::GetName() const
{
	static const string name = "Lux PNG-Loader";
	return name;
}

core::Name ImageLoaderPNG::GetResourceType(io::File* file, core::Name requestedType)
{
	if(requestedType && requestedType != core::ResourceType::Image)
		return core::Name::INVALID;

	u8 bytes[8];
	u32 readBytes = file->ReadBinary(sizeof(bytes), bytes);
	if(readBytes < 8 || !png_check_sig(bytes, sizeof(bytes)))
		return core::Name::INVALID;

	return core::ResourceType::Image;
}

bool ImageLoaderPNG::LoadResource(io::File* file, core::Resource* dst)
{
	bool result;

	video::Image* img = dynamic_cast<video::Image*>(dst);
	if(!img)
		return false;

	result = LoadImageFormat(file);
	if(!result)
		return false;

	img->Init(
		math::dimension2du(m_Width, m_Height),
		m_Format);

	void* data = img->Lock();
	if(!data)
		return false;
	result = LoadImageToMemory(file, data);
	img->Unlock();
	if(!result)
		return false;

	return true;
}

bool ImageLoaderPNG::LoadImageFormat(io::File* file)
{
	Init(file);


	// Supress warning about setjmp C++/Objectdeletion, since only trivial object are used in the
	// scope of setjmp.
#pragma warning(suppress: 4611)
	if(setjmp(m_Info.jmpbuf)) {
		Exit();
		return false;
	}

	png_read_info(m_Png, m_PngInfo);

	png_uint_32 width;
	png_uint_32 height;
	int depth;
	int color_type;
	png_get_IHDR(m_Png, m_PngInfo, &width, &height, &depth, &color_type, NULL, NULL, NULL);

	m_Width = width;
	m_Height = height;

	if(color_type == PNG_COLOR_TYPE_GRAY)
		m_Format = ColorFormat::R8G8B8;
	else if(color_type == PNG_COLOR_TYPE_PALETTE) {
		if(png_get_valid(m_Png, m_PngInfo, PNG_INFO_tRNS))
			m_Format = ColorFormat::A8R8G8B8;
		else
			m_Format = ColorFormat::R8G8B8;
	} else if(color_type == PNG_COLOR_TYPE_RGB)
		m_Format = ColorFormat::R8G8B8;
	else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		m_Format = ColorFormat::A8R8G8B8;
	else if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		m_Format = ColorFormat::A8R8G8B8;
	else {
		if(png_get_valid(m_Png, m_PngInfo, PNG_INFO_tRNS))
			m_Format = ColorFormat::A8R8G8B8;
		else
			m_Format = ColorFormat::R8G8B8;
	}

	m_Depth = depth;
	m_ColorType = color_type;

	return true;
}

bool ImageLoaderPNG::LoadImageToMemory(io::File* file, void* dest)
{
	if(!file)
		return false;
	if(!dest)
		return false;

	if(m_CurFile != file)
		return false;

	// Supress warning about setjmp C++/Objectdeletion, since only trivial object are used in the
	// scope of setjmp.
#pragma warning(suppress: 4611)
	if(setjmp(m_Info.jmpbuf)) {
		Exit();
		return false;
	}

	if(m_ColorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(m_Png);
	if(m_ColorType == PNG_COLOR_TYPE_GRAY && m_Depth < 8)
		png_set_expand_gray_1_2_4_to_8(m_Png);
	if(png_get_valid(m_Png, m_PngInfo, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(m_Png);

	if(m_Depth == 16)
		png_set_strip_16(m_Png);
	if(m_ColorType == PNG_COLOR_TYPE_GRAY ||
		m_ColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(m_Png);

	if(m_Format.HasAlpha()) {
		png_set_bgr(m_Png);
	}

	if(!m_Format.HasAlpha())
		png_set_strip_alpha(m_Png);

	png_read_update_info(m_Png, m_PngInfo);

	png_size_t rowbytes;
	int channels;

	rowbytes = png_get_rowbytes(m_Png, m_PngInfo);
	channels = (int)png_get_channels(m_Png, m_PngInfo);

	u32 pitchLux = m_Format.GetBytePerPixel() * m_Width;
	if(rowbytes != pitchLux) {
		Exit();
		return false;
	}

	png_bytepp row_pointers = (png_bytepp)alloca(m_Height * sizeof(png_bytep));
	for(u32 i = 0; i < m_Height; ++i)
		row_pointers[i] = (png_bytep)dest + i * rowbytes;

	png_read_image(m_Png, row_pointers);

	Exit();

	return true;
}

void ImageLoaderPNG::Init(io::File* file)
{
	if(!file)
		return;

	if(file == m_CurFile)
		return;
	m_CurFile = nullptr;

	Exit();

	m_Png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		&m_Info, png_error_handler, png_warning_handler);
	if(!m_Png)
		return;

	m_PngInfo = png_create_info_struct(m_Png);
	if(!m_PngInfo) {
		png_destroy_read_struct(&m_Png, NULL, NULL);
		return;
	}
	png_set_read_fn(m_Png, file, &png_read_function);

	m_CurFile = file;
}

void ImageLoaderPNG::Exit()
{
	png_destroy_read_struct(&m_Png, &m_PngInfo, NULL);
	m_Png = nullptr;
	m_PngInfo = nullptr;
}

}
}