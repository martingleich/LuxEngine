#ifndef INCLUDED_TGA_H
#define INCLUDED_TGA_H
#include "tgaconfig.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	TGA_OK = 0,
	TGA_ERROR,
	TGA_OUT_OF_MEMORY,
	TGA_IO_ERROR,
	TGA_UNKNOWN_FORMAT,
	TGA_INVALID_ARG,
	TGA_ERROR_COUNT,
};

enum {
	TGA_FLAG_RLE = 1,

	TGA_FLAG_COLOR_RGB = 4,
	TGA_FLAG_COLOR_BGR = 8,

	TGA_FLAG_ALPHA = 16,

	TGA_FLAG_BOTTOM = 32,
	TGA_FLAG_RIGHT = 64,
};

typedef struct tga_struct_t tga_struct;
typedef struct tga_info_t tga_info;

typedef void (*tga_error_proc)(tga_struct*, int, const char*);

// Seeks to the position offset, return 0 in the case of success othwise not zero
typedef uint32_t (*tga_seek_proc)(tga_struct*, uint32_t offset);
// Reads count bytes and writes the to dst, returns total number of bytes written
typedef uint32_t (*tga_read_proc)(tga_struct*, uint32_t count, void* dst);
typedef uint32_t (*tga_write_proc)(tga_struct*, uint32_t, void*);

#ifdef __cplusplus
extern "C" {
#endif

tga_struct* tga_init_read(tga_error_proc error);
tga_struct* tga_init_write(tga_error_proc error);

void tga_destroy(tga_struct* tga);
void tga_destroy_info(tga_info* info);

uint32_t tga_init_io(tga_struct* tga, FILE* fp);

uint32_t tga_set_io_data(tga_struct* tga, void* data);
uint32_t tga_set_io_read_proc(tga_struct* tga, tga_read_proc proc);
uint32_t tga_set_io_write_proc(tga_struct* tga, tga_write_proc proc);
uint32_t tga_set_io_seek_proc(tga_struct* tga, tga_seek_proc proc);
uint32_t tga_set_silent(tga_struct* tga, uint32_t silent);

void* tga_get_io_data(tga_struct* tga);
tga_read_proc tga_get_io_read_proc(tga_struct* tga);
tga_write_proc tga_get_io_write_proc(tga_struct* tga);
tga_seek_proc tga_get_io_seek_proc(tga_struct* tga);
uint32_t ga_get_silent(tga_struct* tga);

tga_info* tga_init_info(tga_struct* tga);

uint32_t tga_info_set_width(tga_info* info, uint32_t width);
uint32_t tga_info_set_height(tga_info* info, uint32_t height);
uint32_t tga_info_set_flags(tga_info* info, uint32_t flags);
uint32_t tga_info_set_background_color(tga_info* info, uint32_t background);

uint32_t tga_info_get_width(tga_info* info);
uint32_t tga_info_get_height(tga_info* info);
uint32_t tga_info_get_flags(tga_info* info);
uint32_t tga_info_get_background_color(tga_info* info);

uint32_t tga_info_set_pitch(tga_info* info, uint32_t pitch);
uint32_t tga_info_get_pitch(tga_info* info);

uint32_t tga_is_info_valid(tga_info* info);

uint32_t tga_is_valid_tga(tga_struct* tga);

uint32_t tga_read_info(tga_struct* tga, tga_info* info);
uint32_t tga_read_image(tga_struct* tga, tga_info* info, void* data);

uint32_t tga_write_image(tga_struct* tga, tga_info* info, void* data);

#ifdef __cplusplus
}
#endif

#endif