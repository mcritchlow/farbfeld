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
	size_t rowlen;
	uint32_t hdr[4], width, height, i;
	uint16_t *row;

	argv0 = argv[0], argc--, argv++;

	if (argc) {
		fprintf(stderr, "usage: %s\n", argv0);
		return 1;
	}

	/* header */
	if (fread(hdr, sizeof(*hdr), 4, stdin) != 4) {
		goto readerr;
	}
	if (memcmp("farbfeld", hdr, sizeof("farbfeld") - 1)) {
		fprintf(stderr, "%s: invalid magic value\n", argv0);
		return 1;
	}
	width = ntohl(hdr[2]);
	height = ntohl(hdr[3]);

	/* load png */
	pngs = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, pngerr,
	                               NULL);
	pngi = png_create_info_struct(pngs);

	if (!pngs || !pngi) {
		fprintf(stderr, "%s: failed to initialize libpng\n", argv0);
		return 1;
	}
	png_init_io(pngs, stdout);
	png_set_IHDR(pngs, pngi, width, height, 16, PNG_COLOR_TYPE_RGB_ALPHA,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
	             PNG_FILTER_TYPE_BASE);
	png_write_info(pngs, pngi);

	/* write rows */
	if (width > SIZE_MAX / ((sizeof("RGBA") - 1) * sizeof(uint16_t))) {
		fprintf(stderr, "%s: row length integer overflow\n", argv0);
		return 1;
	}
	rowlen = width * (sizeof("RGBA") - 1);
	if (!(row = malloc(rowlen * sizeof(uint16_t)))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv0);
		return 1;
	}
	for (i = 0; i < height; ++i) {
		if (fread(row, sizeof(uint16_t), rowlen, stdin) != rowlen) {
			goto readerr;
		}
		png_write_row(pngs, (uint8_t *)row);
	}
	png_write_end(pngs, NULL);
	png_destroy_write_struct(&pngs, NULL);

	return 0;
readerr:
	if (ferror(stdin)) {
		fprintf(stderr, "%s: fread: %s\n", argv0, strerror(errno));
	} else {
		fprintf(stderr, "%s: unexpected end of file\n", argv0);
	}

	return 1;
}
