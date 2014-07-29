/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>
#include "arg.h"

char *argv0;

static void
usage(void)
{
	fprintf(stderr, "usage: %s\n", argv0);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	png_structp png_struct_p;
	png_infop png_info_p;
	png_bytepp png_row_p;
	png_uint_32 width, height, i;
	png_size_t png_row_len;
	uint32_t val_be;
	int depth, color, interlace;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc != 0)
		usage();

	/* load png */
	png_struct_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_info_p = png_create_info_struct(png_struct_p);

	if (!png_struct_p || !png_info_p || setjmp(png_jmpbuf(png_struct_p))) {
		fprintf(stderr, "failed to initialize libpng");
		return EXIT_FAILURE;
	}
	png_init_io(png_struct_p, stdin);
	png_set_add_alpha(png_struct_p, 255, PNG_FILLER_AFTER);
	png_set_gray_to_rgb(png_struct_p);
	png_read_png(png_struct_p, png_info_p, PNG_TRANSFORM_STRIP_16 |
		     PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND , NULL);
	png_get_IHDR(png_struct_p, png_info_p, &width, &height, &depth,
		     &color, &interlace, NULL, NULL);
	png_row_len = png_get_rowbytes(png_struct_p, png_info_p);
	png_row_p = png_get_rows(png_struct_p, png_info_p);

	/* write header with big endian width and height-values */
	fprintf(stdout, "imagefile");
	val_be = htonl(width);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);
	val_be = htonl(height);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);

	/* write data */
	for (i = 0; i < height; i++) {
		if (fwrite(png_row_p[i], 1, png_row_len, stdout) != png_row_len) {
			fprintf(stderr, "fwrite() failed\n");
			return EXIT_FAILURE;
		}
	}

	/* clean up */
	png_destroy_read_struct(&png_struct_p, &png_info_p, NULL);
	return EXIT_SUCCESS;
}
