/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <png.h>

#include "util.h"

void
pngerr(png_structp pngs, const char *msg)
{
	fprintf(stderr, "%s: libpng: %s\n", argv0, msg);
	exit(1);
}

int
main(int argc, char *argv[])
{
	png_structp pngs;
	png_infop pngi;
	uint32_t width, height, rowlen, r, i;
	uint16_t *row;
	uint8_t **pngrows;

	argv0 = argv[0], argc--, argv++;

	if (argc) {
		fprintf(stderr, "usage: %s\n", argv0);
		return 1;
	}

	/* load png */
	pngs = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, pngerr,
	                              NULL);
	pngi = png_create_info_struct(pngs);

	if (!pngs || !pngi) {
		fprintf(stderr, "%s: failed to initialize libpng\n", argv0);
		return 1;
	}
	png_init_io(pngs, stdin);
	if (png_get_valid(pngs, pngi, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(pngs);
	png_set_add_alpha(pngs, 255*257, PNG_FILLER_AFTER);
	png_set_expand_gray_1_2_4_to_8(pngs);
	png_set_gray_to_rgb(pngs);
	png_set_packing(pngs);
	png_read_png(pngs, pngi, PNG_TRANSFORM_PACKING |
	             PNG_TRANSFORM_EXPAND, NULL);
	width = png_get_image_width(pngs, pngi);
	height = png_get_image_height(pngs, pngi);
	pngrows = png_get_rows(pngs, pngi);

	/* allocate output row buffer */
	if (width > SIZE_MAX / ((sizeof("RGBA") - 1) * sizeof(uint16_t))) {
		fprintf(stderr, "%s: row length integer overflow\n", argv0);
		return 1;
	}
	rowlen = width * (sizeof("RGBA") - 1);
	if (!(row = malloc(rowlen * sizeof(uint16_t)))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv0);
		return 1;
	}

	/* write data */
	write_ff_header(width, height);

	switch(png_get_bit_depth(pngs, pngi)) {
	case 8:
		for (r = 0; r < height; ++r) {
			for (i = 0; i < rowlen; i++) {
				row[i] = htons(257 * pngrows[r][i]);
			}
			if (fwrite(row, sizeof(uint16_t), rowlen,
			           stdout) != rowlen) {
				goto writerr;
			}
		}
		break;
	case 16:
		for (r = 0; r < height; ++r) {
			if (fwrite(pngrows[r], sizeof(uint16_t), rowlen,
			           stdout) != rowlen) {
				goto writerr;
			}
		}
		break;
	default:
		fprintf(stderr, "%s: invalid bit-depth\n", argv0);
		return 1;
	}

	png_destroy_read_struct(&pngs, &pngi, NULL);

	return 0;
writerr:
	fprintf(stderr, "%s: fwrite: ", argv0);
	perror(NULL);

	return 1;
}
