/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

static char *argv0;

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
	uint32_t width, height, outrowlen, tmp32, r, i;
	uint16_t *outrow;
	uint8_t **png_row_p;

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
	png_row_p = png_get_rows(pngs, pngi);

	/* allocate output row buffer */
	outrowlen = width * strlen("RGBA");
	if (!(outrow = malloc(outrowlen * sizeof(uint16_t)))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv0);
		return 1;
	}

	/* write header */
	fputs("farbfeld", stdout);
	tmp32 = htonl(width);
	if (fwrite(&tmp32, sizeof(uint32_t), 1, stdout) != 1)
		goto writerr;
	tmp32 = htonl(height);
	if (fwrite(&tmp32, sizeof(uint32_t), 1, stdout) != 1)
		goto writerr;

	/* write data */
	switch(png_get_bit_depth(pngs, pngi)) {
	case 8:
		for (r = 0; r < height; ++r) {
			for (i = 0; i < outrowlen; i++) {
				outrow[i] = htons(257 * png_row_p[r][i]);
			}
			if (fwrite(outrow, sizeof(uint16_t), outrowlen,
			           stdout) != outrowlen) {
				goto writerr;
			}
		}
		break;
	case 16:
		for (r = 0; r < height; ++r) {
			for (i = 0; i < outrowlen; ++i) {
				outrow[i] = ((uint16_t *)png_row_p[r])[i];
			}
			if (fwrite(outrow, sizeof(uint16_t), outrowlen,
			           stdout) != outrowlen) {
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
