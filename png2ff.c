/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "util.h"

static void
png_err(png_struct *pngs, const char *msg)
{
	(void)pngs;
	fprintf(stderr, "%s: libpng: %s\n", argv0, msg);
	exit(1);
}

static void
png_setup_reader(png_struct **s, png_info **i, uint32_t *w, uint32_t *h)
{
	*s = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, png_err, NULL);
	*i = png_create_info_struct(*s);

	if (!*s || !*i) {
		fprintf(stderr, "%s: failed to initialize libpng\n", argv0);
		exit(1);
	}

	png_init_io(*s, stdin);
	if (png_get_valid(*s, *i, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(*s);
	}
	png_set_add_alpha(*s, 255*257, PNG_FILLER_AFTER);
	png_set_expand_gray_1_2_4_to_8(*s);
	png_set_gray_to_rgb(*s);
	png_set_packing(*s);
	png_read_png(*s, *i, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
	*w = png_get_image_width(*s, *i);
	*h = png_get_image_height(*s, *i);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	png_struct *pngs;
	png_info *pngi;
	uint32_t width, height, rowlen, r, i;
	uint16_t *row;
	uint8_t **pngrows;

	/* arguments */
	argv0 = argv[0], argc--, argv++;

	if (argc) {
		usage();
	}

	/* prepare */
	png_setup_reader(&pngs, &pngi, &width, &height);
	row = ereallocarray(NULL, width, (sizeof("RGBA") - 1) * sizeof(uint16_t));
	rowlen = width * (sizeof("RGBA") - 1);
	pngrows = png_get_rows(pngs, pngi);

	/* write data */
	ff_write_header(width, height);

	switch(png_get_bit_depth(pngs, pngi)) {
	case 8:
		for (r = 0; r < height; ++r) {
			for (i = 0; i < rowlen; i++) {
				row[i] = htons(257 * pngrows[r][i]);
			}
			if (fwrite(row, sizeof(uint16_t), rowlen,
			           stdout) != rowlen) {
				fprintf(stderr, "%s: fwrite: %s\n", argv0,
				        strerror(errno));
				return 1;
			}
		}
		break;
	case 16:
		for (r = 0; r < height; ++r) {
			if (fwrite(pngrows[r], sizeof(uint16_t), rowlen,
			           stdout) != rowlen) {
				fprintf(stderr, "%s: fwrite: %s\n", argv0,
				        strerror(errno));
				return 1;
			}
		}
		break;
	default:
		fprintf(stderr, "%s: invalid bit-depth\n", argv0);
		return 1;
	}

	/* clean up */
	png_destroy_read_struct(&pngs, &pngi, NULL);

	return fshut(stdout, "<stdout>");
}
