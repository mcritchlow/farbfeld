/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg.h"

char *argv0;

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-b #rrggbb]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	size_t rowlen;
	uint32_t hdr[4], width, height, i, j, k;
	uint16_t *row, mr = 0xffff, mg = 0xffff, mb = 0xffff;
	uint8_t *rowout;
	char *color;
	unsigned int r = 0xff, g = 0xff, b = 0xff;
	float a;

	argv0 = argv[0];
	ARGBEGIN {
	case 'b':
		for (color = EARGF(usage()); *color && *color == '#'; color++)
			;

		switch (strlen(color)) {
		case 3:
			if (sscanf(color, "%1x%1x%1x", &r, &g, &b) != 3)
				usage();
			mr = (r | r << 4) * 257;
			mg = (g | g << 4) * 257;
			mb = (b | b << 4) * 257;
			break;
		case 6:
			if (sscanf(color, "%2x%2x%2x", &r, &g, &b) != 3)
				usage();
			mr = r * 257;
			mg = g * 257;
			mb = b * 257;
			break;
		case 12:
			if (sscanf(color, "%4x%4x%4x", &r, &g, &b) != 3)
				usage();
			mr = r;
			mg = g;
			mb = b;
			break;
		default:
			usage();
		}
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
	if (!(width = ntohl(hdr[2]))) {
		fprintf(stderr, "%s: invalid width: zero\n", argv0);
		return 1;
	}
	if (!(height = ntohl(hdr[3]))) {
		fprintf(stderr, "%s: invalid height: zero\n", argv0);
		return 1;
	}
	if (width > SIZE_MAX / ((sizeof("RGBA") - 1) * sizeof(uint16_t))) {
		fprintf(stderr, "%s: row length integer overflow\n", argv0);
		return 1;
	}

	rowlen = width * (sizeof("RGBA") - 1);
	if (!(row = malloc(rowlen * sizeof(uint16_t)))) {
		fprintf(stderr, "%s: malloc: %s\n", argv0, strerror(errno));
		return 1;
	}
	if (!(rowout = malloc(width * sizeof("RGB") - 1))) {
		fprintf(stderr, "%s: malloc: %s\n", argv0, strerror(errno));
		return 1;
	}

	/* PPM binary */
	printf("P6\n%" PRIu32 " %" PRIu32 "\n255\n", width, height);

	/* write rows */
	for (i = 0; i < height; ++i) {
		if (fread(row, sizeof(uint16_t), rowlen, stdin) != rowlen) {
			fprintf(stderr, "%s: fread: %s\n", argv0, strerror(errno));
			return 1;
		}
		for (j = 0, k = 0; j < rowlen; j += 4, k += 3) {
			a = ntohs(row[j + 3]) / 65535.0f;
			rowout[k]     = ((ntohs(row[j]) * a)     + (mr * (1 - a))) / 257;
			rowout[k + 1] = ((ntohs(row[j + 1]) * a) + (mg * (1 - a))) / 257;
			rowout[k + 2] = ((ntohs(row[j + 2]) * a) + (mb * (1 - a))) / 257;
		}
		if (fwrite(rowout, 3, width, stdout) != width) {
			fprintf(stderr, "%s: fwrite: %s\n", argv0, strerror(errno));
			return 1;
		}
	}
	return 0;
}
