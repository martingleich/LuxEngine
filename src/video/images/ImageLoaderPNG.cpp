#include "ImageLoaderPNG.h"
#include "video/images/Image.h"
#include "io/File.h"
#include "math/dimension2d.h"
#include "video/Color.h"
#include "../external/libpng/src/png.h"

namespace lux
{
namespace video
{

namespace
{
struct Context
{
	jmp_buf jmpbuf;

	png_struct* png;
	png_info* pngInfo;

	int depth;
	int colorType;
	math::dimension2du size;
	ColorFormat format;

	Context() :
		png(nullptr),
		pngInfo(nullptr)
	{}
	
	~Context()
	{
		png_destroy_read_struct(&png, &pngInfo, NULL);
	}
};
}

static void png_read_function(png_structp png_ptr, png_bytep data, png_size_t count)
{
	io::File* f = (io::File*)png_get_io_ptr(png_ptr);
	if(!f)
		png_error(png_ptr, "No read file defined");

	if(count != (png_size_t)f->ReadBinary((u32)count, data))
		png_error(png_ptr, "Unexpected end of file");
}

static void png_error_handler(png_structp png_ptr, png_const_charp msg)
{
	LUX_UNUSED(msg);
	Context* ctx = (Context*)png_get_error_ptr(png_ptr);
	longjmp(ctx->jmpbuf, 1);
}

static void png_warning_handler(png_structp png_ptr, png_const_charp msg)
{
	LUX_UNUSED(png_ptr);
	LUX_UNUSED(msg);
}

static bool LoadImageFormat(Context& ctx)
{
	// Supress warning about setjmp C++/Objectdeletion, since only trivial object are used in the
	// scope of setjmp.
#pragma warning(suppress: 4611)
	if(setjmp(ctx.jmpbuf)) {
		return false;
	}

	png_read_info(ctx.png, ctx.pngInfo);

	png_uint_32 width;
	png_uint_32 height;
	int depth;
	int color_type;
	png_get_IHDR(ctx.png, ctx.pngInfo, &width, &height, &depth, &color_type, NULL, NULL, NULL);

	ctx.size.Set(width, height);

	if(color_type == PNG_COLOR_TYPE_GRAY)
		ctx.format = ColorFormat::R8G8B8;
	else if(color_type == PNG_COLOR_TYPE_PALETTE) {
		if(png_get_valid(ctx.png, ctx.pngInfo, PNG_INFO_tRNS))
			ctx.format = ColorFormat::A8R8G8B8;
		else
			ctx.format = ColorFormat::R8G8B8;
	} else if(color_type == PNG_COLOR_TYPE_RGB)
		ctx.format = ColorFormat::R8G8B8;
	else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		ctx.format = ColorFormat::A8R8G8B8;
	else if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		ctx.format = ColorFormat::A8R8G8B8;
	else {
		if(png_get_valid(ctx.png, ctx.pngInfo, PNG_INFO_tRNS))
			ctx.format = ColorFormat::A8R8G8B8;
		else
			ctx.format = ColorFormat::R8G8B8;
	}

	ctx.depth = depth;
	ctx.colorType = color_type;

	return true;
}

static bool LoadImageToMemory(Context& ctx, void* dest)
{
	if(!dest)
		return false;

	// Supress warning about setjmp C++/Objectdeletion, since only trivial object are used in the
	// scope of setjmp.
#pragma warning(suppress: 4611)
	if(setjmp(ctx.jmpbuf)) {
		return false;
	}

	if(ctx.colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(ctx.png);
	if(ctx.colorType == PNG_COLOR_TYPE_GRAY && ctx.depth < 8)
		png_set_expand_gray_1_2_4_to_8(ctx.png);
	if(png_get_valid(ctx.png, ctx.pngInfo, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(ctx.png);

	if(ctx.depth == 16)
		png_set_strip_16(ctx.png);
	if(ctx.colorType == PNG_COLOR_TYPE_GRAY ||
		ctx.colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(ctx.png);

	if(ctx.format.HasAlpha()) {
		png_set_bgr(ctx.png);
	}

	if(!ctx.format.HasAlpha())
		png_set_strip_alpha(ctx.png);

	png_read_update_info(ctx.png, ctx.pngInfo);

	png_size_t rowbytes;
	int channels;

	rowbytes = png_get_rowbytes(ctx.png, ctx.pngInfo);
	channels = (int)png_get_channels(ctx.png, ctx.pngInfo);

	u32 pitchLux = ctx.format.GetBytePerPixel() * ctx.size.width;
	if(rowbytes != pitchLux) {
		return false;
	}

	png_bytepp row_pointers = (png_bytepp)alloca(ctx.size.height * sizeof(png_bytep));
	for(u32 i = 0; i < ctx.size.height; ++i)
		row_pointers[i] = (png_bytep)dest + i * rowbytes;

	png_read_image(ctx.png, row_pointers);

	return true;
}

static bool Init(Context& ctx, io::File* file)
{
	if(!file)
		return false;

	ctx.png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		&ctx, png_error_handler, png_warning_handler);
	if(!ctx.png)
		return false;

	ctx.pngInfo = png_create_info_struct(ctx.png);
	if(!ctx.pngInfo) {
		png_destroy_read_struct(&ctx.png, NULL, NULL);
		return false;
	}
	png_set_read_fn(ctx.png, file, &png_read_function);

	return true;
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

	// Context is destroyed at end of scope.
	Context ctx;
	if(!Init(ctx, file))
		return false;

	if(!LoadImageFormat(ctx))
		return false;

	img->Init(ctx.size, ctx.format);

	void* data = img->Lock();
	if(!data)
		return false;

	result = LoadImageToMemory(ctx, data);
	img->Unlock();
	if(!result)
		return false;

	return true;
}

}
}