/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "arg.h"
#include <png.h>

char *argv0;

#define HEADER_FORMAT "imagefile########"

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
	uint8_t hdr[17], *png_row;
	uint32_t width, height, png_row_len, i;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc != 0)
		usage();

	/* header */
	if (fread(hdr, 1, strlen(HEADER_FORMAT), stdin) != strlen(HEADER_FORMAT)) {
		fprintf(stderr, "failed to read from stdin or input too short\n");
		goto err;
	}
	if (strcmp("imagefile", hdr)) {
		fprintf(stderr, "invalid header\n");
		goto err;
	}
	width = ntohl((hdr[9] << 0) | (hdr[10] << 8) | (hdr[11] << 16) | (hdr[12] << 24));
	height = ntohl((hdr[13] << 0) | (hdr[14] << 8) | (hdr[15] << 16) | (hdr[16] << 24));

	/* load png */
	png_struct_p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_info_p = png_create_info_struct(png_struct_p);

	if (!png_struct_p || !png_info_p || setjmp(png_jmpbuf(png_struct_p))) {
		fprintf(stderr, "failed to initialize libpng\n");
		goto err;
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
		goto err;
	}

	for (i=0; i < height; ++i) {
		if (fread(png_row, 1, png_row_len, stdin) != png_row_len) {
			fprintf(stderr, "unexpected EOF or row-skew at %d\n", i);
			goto err;
		}
		png_write_row(png_struct_p, png_row);
	}
	png_write_end(png_struct_p, NULL);

	/* clean up */
	png_free_data(png_struct_p, png_info_p, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_struct_p, NULL);
	free(png_row);
	return EXIT_SUCCESS;
err:
	png_free_data(png_struct_p, png_info_p, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_struct_p, NULL);
	free(png_row);
	return EXIT_FAILURE;
}
