#include "tga.h"
#include <string>
#include <assert.h>

typedef struct tga_header_t tga_header;
struct tga_header_t
{
	uint8_t id_length; // image id length
	uint8_t map_type; // color map type
	uint8_t img_type; // image type
	uint16_t map_first; // index of first map entry
	uint16_t map_length; // number of entries in the color map
	uint8_t map_entry; // bit-depth of a color map entry
	uint16_t x; // x coordinate
	uint16_t y; // y coordinate
	uint16_t width; // width of image
	uint16_t height; // height of image
	uint8_t depth; // bit-depth of image
	uint8_t alpha; // alpha bits
	uint8_t vert; // horizontal orientation
	uint8_t horz; // horzical orientation
};

struct tga_struct_t
{
	uint8_t mode;

	tga_header header;
	uint32_t has_header;

	tga_error_proc error_proc;

	FILE* file;
	tga_seek_proc io_seek_proc;
	tga_read_proc io_read_proc;
	tga_write_proc io_write_proc;
	void* io_data;
	uint32_t file_offset;

	uint32_t last_error;

	uint8_t* temp_block;
	uint32_t temp_size;

	uint32_t silent;
};

struct tga_info_t
{
	tga_struct* tga;

	uint32_t width;
	uint32_t height;

	uint32_t pitch;

	uint32_t flags;

	uint32_t background_color; // Hintergrundfarbe als A8R8G8B8
};

#define TGA_MODE_READ 1
#define TGA_MODE_WRITE 2

#define TGA_HEADER_SIZE 18

static const char* tga_error_str(int error)
{
	static const char* error_string[TGA_ERROR_COUNT] = {"Success", "Error", "Out of memory", "IO Error", "Unknown format", "Invalid argument"};

	if(error >= TGA_ERROR_COUNT)
		return error_string[TGA_ERROR];
	else
		return error_string[error];
}

static void tga_report_error(tga_struct* tga, int code, const char* file, int line)
{
	if(tga && tga->error_proc)
		tga->error_proc(tga, code, tga_error_str(code));
	else if(!TGA_NO_ERROR_MSG && !(tga && tga->silent))
		fprintf(stderr, "libtga:%s:%d %s\n", file, line, tga_error_str(code));

	if(tga)
		tga->last_error = code;
}

#define TGA_REPORT_ERROR(tga, code) do { tga_report_error(tga, code, __FILE__, __LINE__); } while(false)
#define TGA_CHECK_NULL(tga, value, ret) do{if(!value) { tga_report_error(tga, TGA_INVALID_ARG, __FILE__, __LINE__); return ret;}} while(false)

#define tga_set_get(type, name) \
uint32_t tga_set_##name(tga_struct* tga, type name) { \
	TGA_CHECK_NULL(tga, tga, TGA_INVALID_ARG); \
	tga->name = name; \
	return TGA_OK; \
} \
type tga_get_##name(tga_struct* tga) { \
	TGA_CHECK_NULL(tga, tga, 0); \
	return tga->name; \
}

#define tga_info_set_get(type, name) \
uint32_t tga_info_set_##name(tga_info* info, type name) { \
	TGA_CHECK_NULL((tga_struct*)0, info, TGA_INVALID_ARG); \
	info->name = name; \
	return TGA_OK; \
} \
type tga_info_get_##name(tga_info* info) { \
	TGA_CHECK_NULL((tga_struct*)0, info, 0); \
	return info->name; \
}

#define TGA_SWAP(a, b, tmp) do{tmp=a;a=b;b=tmp;}while(false);

static inline uint32_t tga_get_result_bits(tga_struct* tga, tga_info* info)
{
	(void)tga;

	if(info->flags & TGA_FLAG_ALPHA)
		return 32;
	else
		return 24;
}

static inline uint32_t tga_get_scanline_size(tga_struct* tga)
{
	assert(tga);
	return tga->header.width * (tga->header.depth / 8);
}

static inline uint32_t tga_get_img_data_offset(tga_struct* tga)
{
	assert(tga);
	return TGA_HEADER_SIZE + tga->header.id_length;
}

static inline uint32_t tga_is_mapped(tga_struct* tga)
{
	assert(tga);
	return (tga->header.map_type == 1);
}

static inline uint32_t tga_is_encoded(tga_struct* tga)
{
	assert(tga);
	return (tga->header.img_type > 8 && tga->header.img_type < 12);
}

static uint8_t* tga_get_temp_memory(tga_struct* tga, uint32_t size)
{
	assert(tga);
	assert(size > 0);
	if(tga->temp_size >= size)
		return tga->temp_block;

	free(tga->temp_block);
	tga->temp_size = 0;
	tga->temp_block = (uint8_t*)malloc(size);
	if(tga->temp_block)
		tga->temp_size = size;

	if(!tga->temp_block) {
		TGA_REPORT_ERROR(tga, TGA_OUT_OF_MEMORY);
		return 0;
	}

	return tga->temp_block;
}

static uint32_t tga_read(tga_struct* tga, uint8_t* buffer, uint32_t size)
{
	assert(tga);
	assert(buffer);
	assert(size > 0);
	assert(tga->mode == TGA_MODE_READ);

	uint32_t read;
	if(tga->io_read_proc)
		read = tga->io_read_proc(tga, size, buffer);
	else
		read = (uint32_t)fread(buffer, size, 1, tga->file);

	if(read > 0)
		tga->file_offset += read;

	return read;
}

static uint32_t tga_write(tga_struct* tga, uint8_t* buffer, uint32_t size)
{
	assert(tga);
	assert(buffer);
	assert(size > 0);
	assert(tga->mode == TGA_MODE_WRITE);

	uint32_t written;
	if(tga->io_write_proc)
		written = tga->io_write_proc(tga, size, buffer);
	else
		written = (uint32_t)fwrite(buffer, size, 1, tga->file);

	if(written > 0)
		tga->file_offset += written;

	return written;
}

static uint32_t tga_seek(tga_struct* tga, uint32_t off)
{
	assert(tga);

	if(tga->io_seek_proc)
		return tga->io_seek_proc(tga, off);
	else
		return fseek(tga->file, off, SEEK_SET);
}

static tga_struct* tga_init_base(tga_error_proc error)
{
	tga_struct* tga = (tga_struct*)malloc(sizeof(tga_struct));
	if(!tga) {
		TGA_REPORT_ERROR(tga, TGA_OUT_OF_MEMORY);
		return 0;
	}

	tga->error_proc = error;
	tga->file = NULL;
	tga->io_seek_proc = NULL;
	tga->io_read_proc = NULL;
	tga->io_write_proc = NULL;
	tga->file_offset = 0;
	tga->last_error = TGA_OK;
	tga->has_header = 0;
	tga->temp_block = NULL;
	tga->temp_size = 0;
	tga->silent = 0;

	return tga;
}

tga_struct* tga_init_read(tga_error_proc error)
{
	tga_struct* tga = tga_init_base(error);
	if(!tga)
		return 0;

	tga->mode = TGA_MODE_READ;

	return tga;
}

tga_struct* tga_init_write(tga_error_proc error)
{
	tga_struct* tga = tga_init_base(error);
	if(!tga)
		return 0;

	tga->mode = TGA_MODE_WRITE;

	return tga;
}

void tga_destroy(tga_struct* tga)
{
	if(!tga)
		return;

	free(tga->temp_block);
	free(tga);
}

void tga_destroy_info(tga_info* info)
{
	free(info);
}

uint32_t tga_init_io(tga_struct* tga, FILE* fp)
{
	TGA_CHECK_NULL(tga, tga, TGA_INVALID_ARG);
	TGA_CHECK_NULL(tga, fp, TGA_INVALID_ARG);

	long off = ftell(fp);
	if(off == -1) {
		TGA_REPORT_ERROR(tga, TGA_IO_ERROR);
		return tga->last_error;
	}

	tga->file_offset = (uint32_t)off;

	return TGA_OK;
}

tga_set_get(void*, io_data)
tga_set_get(tga_read_proc, io_read_proc)
tga_set_get(tga_write_proc, io_write_proc)
tga_set_get(tga_seek_proc, io_seek_proc)
tga_set_get(uint32_t, silent);

tga_info* tga_init_info(tga_struct* tga)
{
	TGA_CHECK_NULL(tga, tga, 0);

	tga_info* info = (tga_info*)malloc(sizeof(tga_info));
	if(!info) {
		TGA_REPORT_ERROR(tga, TGA_OUT_OF_MEMORY);
		return 0;
	}

	info->tga = tga;
	info->background_color = 0;
	info->height = 0;
	info->width = 0;
	info->flags = TGA_FLAG_COLOR_RGB;
	info->pitch = 0;

	return info;
}

tga_info_set_get(uint32_t, width)
tga_info_set_get(uint32_t, height)
tga_info_set_get(uint32_t, pitch);
tga_info_set_get(uint32_t, flags);
tga_info_set_get(uint32_t, background_color)

uint32_t tga_is_info_valid(tga_info* info)
{
	TGA_CHECK_NULL((tga_struct*)0, info, TGA_INVALID_ARG);
	TGA_CHECK_NULL(info->tga, info->tga, TGA_INVALID_ARG);

	if(info->width > 0xFFFF || info->height > 0xFFFF)
		return 0;

	if(info->tga->mode == TGA_MODE_READ) {
		if(info->width != info->tga->header.width)
			return 0;
		if(info->height != info->tga->header.height)
			return 0;
		if(info->flags & TGA_FLAG_BOTTOM)
			return 0;
		if(info->flags & TGA_FLAG_RIGHT)
			return 0;
		if(info->flags & TGA_FLAG_RLE)
			return 0;
		if(info->pitch != 0) {
			return 0;
		}
	} else if(info->tga->mode == TGA_MODE_WRITE) {
		if(info->width == 0)
			return 0;
		if(info->height == 0)
			return 0;
		if(info->flags & TGA_FLAG_RIGHT)
			return 0;
		if(info->flags & TGA_FLAG_RLE)
			return 0;
		if(info->pitch != 0) {
			if(info->pitch < tga_get_result_bits(info->tga, info) / 8 * info->width)
				return 0;
		}
	} else {
		TGA_REPORT_ERROR(info->tga, TGA_ERROR);
		return 0;
	}

	return 1;
}

static uint32_t tga_read_header(tga_struct* tga)
{
	uint8_t raw_header[TGA_HEADER_SIZE];

	assert(tga);

	if(tga->has_header)
		return TGA_OK;

	if(tga_seek(tga, 0))
		return TGA_IO_ERROR;

	if(tga_read(tga, raw_header, TGA_HEADER_SIZE) != TGA_HEADER_SIZE) {
		TGA_REPORT_ERROR(tga, TGA_IO_ERROR);
		return TGA_IO_ERROR;
	}

	tga->header.id_length = raw_header[0];
	tga->header.map_type = raw_header[1];
	tga->header.img_type = raw_header[2];
	tga->header.map_first = raw_header[3] | raw_header[4] << 8;
	tga->header.map_length = raw_header[5] | raw_header[6] << 8;
	tga->header.map_entry = raw_header[7];
	tga->header.x = raw_header[8] | raw_header[9] << 8;
	tga->header.y = raw_header[10] | raw_header[11] << 8;
	tga->header.width = raw_header[12] | raw_header[13] << 8;
	tga->header.height = raw_header[14] | raw_header[15] << 8;
	tga->header.depth = raw_header[16];
	tga->header.alpha = raw_header[17] & 0x0F;
	tga->header.vert = (raw_header[17] & 0x10) ? TGA_FLAG_RIGHT : 0;
	tga->header.horz = (raw_header[17] & 0x20) ? 0 : TGA_FLAG_BOTTOM;

	// Supported ímage types are:
	// uncompressed mapped
	// uncompressed color
	// run-length color
	if(tga->header.img_type != 1 &&
		tga->header.img_type != 2 &&
		tga->header.img_type != 10) {
	}

	// No color maps
	if(tga->header.map_type == 1) {
		TGA_REPORT_ERROR(tga, TGA_UNKNOWN_FORMAT);
		return TGA_UNKNOWN_FORMAT;
	}

	// Only this bit-depth
	if(tga->header.depth != 8 &&
		tga->header.depth != 15 &&
		tga->header.depth != 16 &&
		tga->header.depth != 24 &&
		tga->header.depth != 32) {
		TGA_REPORT_ERROR(tga, TGA_UNKNOWN_FORMAT);
		return TGA_UNKNOWN_FORMAT;
	}

	// Awful format
	if(tga->header.depth == 15) {
		TGA_REPORT_ERROR(tga, TGA_UNKNOWN_FORMAT);
		return TGA_UNKNOWN_FORMAT;
	}

	// Alpha only with 32-bit depth
	if((tga->header.depth == 15 || tga->header.depth == 16 || tga->header.depth == 24) && tga->header.alpha) {
		TGA_REPORT_ERROR(tga, TGA_UNKNOWN_FORMAT);
		return TGA_UNKNOWN_FORMAT;
	}

	tga->has_header = 1;

	return TGA_OK;
}

uint32_t tga_is_valid_tga(tga_struct* tga)
{
	TGA_CHECK_NULL(tga, tga, TGA_INVALID_ARG);

	if(TGA_OK == tga_read_header(tga))
		return 1;
	else
		return 0;
}

uint32_t tga_read_info(tga_struct* tga, tga_info* info)
{
	TGA_CHECK_NULL(tga, tga, TGA_INVALID_ARG);
	TGA_CHECK_NULL(tga, info, TGA_INVALID_ARG);

	if(info->tga != tga) {
		TGA_REPORT_ERROR(tga, TGA_INVALID_ARG);
		return TGA_INVALID_ARG;
	}

	if(tga_read_header(tga) != TGA_OK)
		return tga->last_error;

	info->width = tga->header.width;
	info->height = tga->header.height;
	info->flags = TGA_FLAG_COLOR_RGB;
	info->pitch = 0;
	if(tga->header.alpha)
		info->flags |= TGA_FLAG_ALPHA;

	return TGA_OK;
}

static void tga_swap_colors(uint8_t* data, uint32_t count, uint32_t bit_per_pixel, uint32_t alpha)
{
	uint8_t tmp;
	uint32_t byte_per_pixel = bit_per_pixel / 8;
	uint8_t* end = data + count*byte_per_pixel;

	if(bit_per_pixel == 32 && alpha) {
		for(; data < end; data += byte_per_pixel) {
			TGA_SWAP(data[0], data[3], tmp);
			TGA_SWAP(data[1], data[2], tmp);
		}
	} else if(bit_per_pixel == 32 && !alpha) {
		for(; data < end; data += byte_per_pixel) {
			TGA_SWAP(data[0], data[2], tmp);
		}
	} else if(bit_per_pixel == 24 && !alpha) {
		for(; data < end; data += byte_per_pixel) {
			TGA_SWAP(data[0], data[2], tmp);
		}
	} else {
		assert(false);
	}
}

static void tga_remove_alpha(uint8_t* data, uint32_t count, uint32_t source_has_alpha, uint32_t result_has_alpha, uint32_t background)
{
	uint8_t* end = data + count * 4;
	uint8_t* dst = data;
	uint8_t r, g, b;

	uint8_t back[3] = {
		(uint8_t)(background & 0xFF),
		(uint8_t)((background >> 8) & 0xFF),
		(uint8_t)((background >> 16) & 0xFF)};

	if(result_has_alpha) {
		for(; data < end; data += 4, dst += 4) {
			if(!source_has_alpha)
				data[3] = 0xFF;
			b = (back[0] * (255 - data[3]) + data[0] * data[3]) / 255;
			g = (back[1] * (255 - data[3]) + data[1] * data[3]) / 255;
			r = (back[2] * (255 - data[3]) + data[2] * data[3]) / 255;
			dst[0] = b;
			dst[1] = g;
			dst[2] = r;
			dst[3] = 0xFF;
		}
	} else {
		for(; data < end; data += 4, dst += 3) {
			if(!source_has_alpha)
				data[3] = 0xFF;
			b = (back[0] * (255 - data[3]) + data[0] * data[3]) / 255;
			g = (back[1] * (255 - data[3]) + data[1] * data[3]) / 255;
			r = (back[2] * (255 - data[3]) + data[2] * data[3]) / 255;
			dst[0] = b;
			dst[1] = g;
			dst[2] = r;
		}
	}
}

static void tga_convert_to_true_color(uint8_t* data, uint32_t count, uint32_t src_bits, uint32_t dst_bits)
{
	uint8_t* cur = data + (count - 1) * (src_bits / 8);
	uint8_t* out_cur = data + count * (dst_bits / 8) - 1;

	assert(data);
	assert(src_bits == 16);
	assert(dst_bits == 24 || dst_bits == 32);

	// 16 = B5G6R5
	// 24 = B8G8R8
	// 32 = B8G8R8A8
	for(; cur >= data; cur -= 2) {
		// 0rrrrrgggggbbbbb
		if(dst_bits == 32)
			*(out_cur--) = 0xFF;

		*(out_cur--) = (*(cur + 1) & 0x7C) << 1;
		*(out_cur--) = (*(cur) & 0x1F) << 3;
		*(out_cur--) = ((*(cur + 1) & 0x03) << 6) | ((*cur & 0xE0) >> 2);
	}
}

static void tga_swap_left_right(uint8_t* buffer, uint32_t count, uint32_t bytes)
{
	uint8_t* front = buffer;
	uint8_t* back = buffer + count*bytes - bytes;
	uint8_t temp[4];

	while(front < back) {
		memcpy(temp, front, bytes);
		memcpy(front, back, bytes);
		memcpy(back, temp, bytes);

		front += bytes;
		back -= bytes;
	}
}

static uint32_t tga_read_rle(tga_struct* tga, uint8_t* buffer)
{
	uint8_t head;
	uint8_t sample[4];
	uint8_t k, repeat = 0, direct = 0, bytes = tga->header.depth / 8;
	uint16_t x;
	uint16_t width = tga->header.width;

	assert(tga);
	assert(buffer);

	for(x = 0; x < width; ++x) {
		if(repeat == 0 && direct == 0) {
			if(tga_read(tga, &head, 1) != 1)
				return TGA_IO_ERROR;
			if(head >= 128) {
				repeat = head - 127;
				if(tga_read(tga, sample, bytes) != bytes)
					return TGA_IO_ERROR;
			} else {
				direct = head + 1;
			}
		}

		if(repeat > 0) {
			for(k = 0; k < bytes; ++k)
				buffer[k] = sample[k];
			--repeat;
		} else {
			if(tga_read(tga, buffer, bytes) != bytes)
				return TGA_IO_ERROR;
			--direct;
		}
		buffer += bytes;
	}

	return TGA_OK;
}

static uint32_t tga_read_scanlines(tga_struct* tga,
	tga_info* info,
	uint8_t* buffer,
	uint32_t scanline,
	uint32_t lines)
{
	uint32_t off, result_bits, scanline_size, read, i;
	uint8_t* buffer_backup;

	assert(tga);
	assert(info);
	assert(buffer);

	scanline_size = tga_get_scanline_size(tga);
	off = tga_get_img_data_offset(tga) + scanline*scanline_size;

	result_bits = tga_get_result_bits(tga, info);

	if(result_bits < tga->header.depth) {
		buffer_backup = buffer;
		buffer = tga_get_temp_memory(tga, tga->header.width*lines*tga->header.depth / 8);
		if(!buffer)
			return tga->last_error;
	} else {
		buffer_backup = nullptr;
	}

	if(tga_seek(tga, off)) {
		TGA_REPORT_ERROR(tga, TGA_IO_ERROR);
		return TGA_IO_ERROR;
	}

	if(tga_is_encoded(tga)) {
		for(read = 0; read <= lines; ++read) {
			if(tga_read_rle(tga, buffer + ((scanline + read)*scanline_size)) != TGA_OK)
				return TGA_ERROR;
		}
	} else {
		if(tga_read(tga, buffer, scanline_size*lines) != scanline_size*lines) {
			TGA_REPORT_ERROR(tga, TGA_IO_ERROR);
			return TGA_IO_ERROR;
		}
	}

	if(tga->header.vert == TGA_FLAG_RIGHT) {
		for(i = 0; i < lines; ++i)
			tga_swap_left_right(buffer + (i * (tga->header.depth / 8) * info->width), info->width, result_bits / 8);
	}

	if(tga->header.depth == 16) {
		// Conhorz to depth 32 or 24, depending on result alpha
		tga_convert_to_true_color(buffer, info->width*lines, tga->header.depth, result_bits);
	}

	if(tga->header.depth == 32 && result_bits == 24) {
		// Remove alpha
		tga_remove_alpha(buffer, info->width*lines, tga->header.alpha, info->flags&TGA_FLAG_ALPHA, info->background_color);
	}

	if(!tga->header.alpha && (info->flags & TGA_FLAG_ALPHA)) {
		// No op, at this there is always a filled or empty alpha channel
	}

	if(info->flags & TGA_FLAG_COLOR_RGB) {
		// Swap channels
		tga_swap_colors(buffer, info->width*lines, result_bits, info->flags&TGA_FLAG_ALPHA);
	}


	if(buffer_backup)
		memcpy(buffer_backup, buffer, result_bits / 8 * lines * info->width);

	return TGA_OK;
}

uint32_t tga_read_image(tga_struct* tga, tga_info* info, void* data)
{
	uint32_t i, result_bits;
	uint8_t* cur;

	if(!tga || !info || !data || info->tga != tga) {
		TGA_REPORT_ERROR(tga, TGA_INVALID_ARG);
		return TGA_INVALID_ARG;
	}

	if(tga_is_mapped(tga)) {
		TGA_REPORT_ERROR(tga, TGA_UNKNOWN_FORMAT);
		return TGA_UNKNOWN_FORMAT;
	}

	result_bits = tga_get_result_bits(tga, info);

	if((tga->header.horz == TGA_FLAG_BOTTOM && !(info->flags & TGA_FLAG_BOTTOM)) ||
		(tga->header.horz != TGA_FLAG_BOTTOM && (info->flags & TGA_FLAG_BOTTOM))) {
		cur = (uint8_t*)data + (info->height - 1) * info->width * result_bits / 8;
		for(i = 0; i < info->height; ++i) {
			if(tga_read_scanlines(tga, info, cur, i, 1))
				return tga->last_error;
			cur -= info->width*result_bits / 8;
		}
	} else {
		if(tga_read_scanlines(tga, info, (uint8_t*)data, 0, tga->header.height)) {
			return tga->last_error;
		}
	}

	return TGA_OK;
}

static uint32_t tga_write_header(tga_struct* tga)
{
	uint8_t raw_header[TGA_HEADER_SIZE];

	assert(tga);

	if(tga->file_offset != 0) {
		if(tga_seek(tga, 0)) {
			TGA_REPORT_ERROR(tga, TGA_IO_ERROR);
			return TGA_IO_ERROR;
		}
	}

	raw_header[0] = tga->header.id_length;
	raw_header[1] = tga->header.map_type;
	raw_header[2] = tga->header.img_type;
	raw_header[3] = (tga->header.map_first & 0x00FF);
	raw_header[4] = (tga->header.map_first & 0xFF00) >> 8;
	raw_header[5] = (tga->header.map_length & 0x00FF);
	raw_header[6] = (tga->header.map_length & 0xFF00) >> 8;
	raw_header[7] = tga->header.map_entry;
	raw_header[8] = (tga->header.x & 0x00FF);
	raw_header[9] = (tga->header.x & 0xFF00) >> 8;
	raw_header[10] = (tga->header.y & 0x00FF);
	raw_header[11] = (tga->header.y & 0xFF00) >> 8;
	raw_header[12] = (tga->header.width & 0x00FF);
	raw_header[13] = (tga->header.width & 0xFF00) >> 8;
	raw_header[14] = (tga->header.height & 0x00FF);
	raw_header[15] = (tga->header.height & 0xFF00) >> 8;
	raw_header[16] = tga->header.depth;
	raw_header[17] = tga->header.alpha;
	raw_header[17] |= (tga->header.vert & TGA_FLAG_RIGHT) ? 0x10 : 0;
	raw_header[17] |= (tga->header.horz & TGA_FLAG_BOTTOM) ? 0 : 0x20;

	if(tga_write(tga, raw_header, sizeof(raw_header)) != sizeof(raw_header)) {
		TGA_REPORT_ERROR(tga, TGA_IO_ERROR);
		return TGA_IO_ERROR;
	}

	return TGA_OK;
}

static uint32_t tga_write_scanline(tga_struct* tga, uint8_t* data, uint32_t count, uint32_t swap)
{
	uint32_t data_size = count * tga->header.depth/8;
	if(swap) {
		uint8_t* swapped = tga_get_temp_memory(tga, data_size);
		if(!swapped)
			return tga->last_error;
		memcpy(swapped, data, count * tga->header.depth/8);
		tga_swap_colors(swapped, count, tga->header.depth, tga->header.alpha);
		data = swapped;
	}

	if(tga_write(tga, data, data_size) != data_size) {
		TGA_REPORT_ERROR(tga, TGA_IO_ERROR);
		return TGA_IO_ERROR;
	}

	return TGA_OK;
}

static uint32_t tga_write_image_data(tga_struct* tga, uint8_t* data, uint32_t pitch, uint32_t swap)
{
	uint32_t offset = tga_get_img_data_offset(tga);
	uint32_t i;

	assert(tga);
	assert(data);
	
	if(pitch == 0)
		pitch = tga_get_scanline_size(tga);

	if(tga->file_offset != offset) {
		if(!tga_seek(tga, offset))
			return TGA_IO_ERROR;
	}

	for(i = 0; i < tga->header.height; ++i) {
		if(tga_write_scanline(tga, data, tga->header.width, swap) != TGA_OK)
			return tga->last_error;

		data += pitch;
	}

	return TGA_OK;
}

uint32_t tga_write_image(tga_struct* tga, tga_info* info, void* data)
{
	TGA_CHECK_NULL(tga, tga, TGA_INVALID_ARG);
	if(!info || info->tga != tga || !data) {
		TGA_REPORT_ERROR(tga, TGA_INVALID_ARG);
		return TGA_INVALID_ARG;
	}
	if(tga_is_info_valid(info) == 0) {
		TGA_REPORT_ERROR(tga, TGA_ERROR);
		return TGA_ERROR;
	}

	tga->header.id_length = 0;
	tga->header.map_type = 0;
	tga->header.width = (uint16_t)info->width; // Checked by tga_is_info_valid
	tga->header.height = (uint16_t)info->height; // Checked by tga_is_info_valid
	tga->header.alpha = (info->flags & TGA_FLAG_ALPHA) ? 8 : 0;
	tga->header.vert = 0;
	tga->header.horz = (info->flags & TGA_FLAG_BOTTOM);
	tga->header.depth = tga->header.alpha ? 32 : 24;
	tga->header.map_entry = 0;
	tga->header.map_length = 0;
	tga->header.x = 0;
	tga->header.y = 0;
	tga->header.img_type = 2;
	
	if(tga_write_header(tga) != TGA_OK)
		return tga->last_error;

	if(tga_write_image_data(tga, (uint8_t*)data, info->pitch, info->flags & TGA_FLAG_COLOR_RGB) != TGA_OK)
		return tga->last_error;

	return TGA_OK;
}
