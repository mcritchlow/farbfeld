/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>

#include "arg.h"

char *argv0;

METHODDEF(void)
jpeg_error(j_common_ptr js)
{
	fprintf(stderr, "%s: libjpeg: ", argv0);
	(*js->err->output_message)(js);
	exit(1);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-b #rrggbb] [-o] [-q quality]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	JSAMPROW row_pointer[1]; /* pointer to a single row */
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	size_t rowlen;
	uint64_t a;
	uint32_t hdr[4], width, height, i, j, k, l;
	uint16_t *row, mask[3] = { 0xffff, 0xffff, 0xffff };
	uint8_t *rowout;
	char *color, colfmt[] = "%#x%#x%#x";
	unsigned int collen, col[3], colfac, quality = 85, optimize = 0;

	argv0 = argv[0];
	ARGBEGIN {
	case 'b':
		color = EARGF(usage());
		if (color[0] == '#') {
			color++;
		}
		collen = strlen(color);
		if (collen != 3 && collen != 6 && collen != 12) {
			usage();
		}
		colfmt[1] = colfmt[4] = colfmt[7] = ((collen / 3) + '0');
		if (sscanf(color, colfmt, col, col + 1, col + 2) != 3) {
			usage();
		}
		/* UINT16_MAX / 255 = 257; UINT16_MAX / 15 = 4369 */
		colfac = (collen == 3) ? 4369 : (collen == 6) ? 257 : 1;
		for (i = 0; i < 3; i++) {
			mask[i] = col[i] * colfac;
		}
		break;
	case 'o':
		optimize = 1;
		break;
	case 'q':
		if ((quality = atoi(EARGF(usage()))) > 100)
			usage();
		break;
	default:
		usage();
	} ARGEND

	if (argc)
		usage();

	/* header */
	if (fread(hdr, sizeof(*hdr), 4, stdin) != 4) {
		fprintf(stderr, "%s: fread: %s\n", argv0, strerror(errno));
		return 1;
	}
	if (memcmp("farbfeld", hdr, sizeof("farbfeld") - 1)) {
		fprintf(stderr, "%s: invalid magic value\n", argv0);
		return 1;
	}
	width = ntohl(hdr[2]);
	height = ntohl(hdr[3]);

	if (width > SIZE_MAX / ((sizeof("RGBA") - 1) * sizeof(uint16_t))) {
		fprintf(stderr, "%s: row length integer overflow\n", argv0);
		return 1;
	}
	rowlen = width * (sizeof("RGBA") - 1);
	if (!(row = malloc(rowlen * sizeof(uint16_t)))) {
		fprintf(stderr, "%s: malloc: %s\n", argv0, strerror(errno));
		return 1;
	}
	if (!(rowout = malloc(width * (sizeof("RGB") - 1) * sizeof(uint8_t)))) {
		fprintf(stderr, "%s: malloc: %s\n", argv0, strerror(errno));
		return 1;
	}
	row_pointer[0] = rowout;

	jerr.error_exit = jpeg_error;

	jpeg_create_compress(&cinfo);
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_stdio_dest(&cinfo, stdout);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;  /* color components per pixel */
	cinfo.in_color_space = JCS_RGB; /* output color space */
	jpeg_set_defaults(&cinfo);
	if (optimize)
		cinfo.optimize_coding = TRUE;
	jpeg_set_quality(&cinfo, quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	/* write rows */
	for (i = 0; i < height; ++i) {
		if (fread(row, sizeof(uint16_t), rowlen, stdin) != rowlen) {
			fprintf(stderr, "%s: fread: %s\n", argv0, strerror(errno));
			return 1;
		}
		for (j = 0, k = 0; j < rowlen; j += 4, k += 3) {
			a = ntohs(row[j + 3]);
			for (l = 0; l < 3; l++) {
				rowout[k + l] = (a * ntohs(row[j + l]) +
				                 (65535 - a) * mask[l]) /
				                (257 * 65535);
			}
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	return 0;
}
