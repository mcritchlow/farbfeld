/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#define HEADER_FORMAT "farbfeld########"

int
main(int argc, char *argv[])
{
	png_structp png_struct_p;
	png_infop png_info_p;
	uint8_t hdr[16], *png_row;
	uint16_t tmp16;
	png_uint_32 width, height, i;
	png_size_t png_row_len, j;

	if (argc > 1) {
		fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}

	/* header */
	if (fread(hdr, 1, strlen(HEADER_FORMAT), stdin) != strlen(HEADER_FORMAT)) {
		fprintf(stderr, "failed to read from stdin or input too short\n");
		return 1;
	}
	if (memcmp("farbfeld", hdr, strlen("farbfeld"))) {
		fprintf(stderr, "invalid magic in header\n");
		return 1;
	}
	width = ntohl(*((uint32_t *)(hdr + 8)));
	height = ntohl(*((uint32_t *)(hdr + 12)));

	/* load png */
	png_struct_p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_info_p = png_create_info_struct(png_struct_p);

	if (!png_struct_p || !png_info_p || setjmp(png_jmpbuf(png_struct_p))) {
		fprintf(stderr, "failed to initialize libpng\n");
		return 1;
	}
	png_init_io(png_struct_p, stdout);
	png_set_IHDR(png_struct_p, png_info_p, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
		     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_struct_p, png_info_p);

	/* write rows */
	png_row_len = strlen("RGBA") * width * sizeof(uint8_t);
	png_row = malloc(png_row_len);
	if (!png_row) {
		fprintf(stderr, "failed to allocate row-buffer\n");
		return 1;
	}

	for (i = 0; i < height; ++i) {
		for (j = 0; j < png_row_len; ++j) {
			if (fread(&tmp16, 1, sizeof(uint16_t), stdin) != sizeof(uint16_t)) {
				fprintf(stderr, "unexpected EOF or row-skew\n");
				return 1;
			}
			/* ((2^16-1) / 255) == 257 */
			png_row[j] = (uint8_t)(ntohs(tmp16) / 257);
		}
		png_write_row(png_struct_p, png_row);
	}
	png_write_end(png_struct_p, NULL);

	/* clean up */
	png_free_data(png_struct_p, png_info_p, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_struct_p, NULL);
	free(png_row);

	return 0;
}
