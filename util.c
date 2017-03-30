/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

char *argv0;

void
read_ff_header(uint32_t *width, uint32_t *height)
{
	uint32_t hdr[4];

	if (fread(hdr, sizeof(*hdr), LEN(hdr), stdin) != LEN(hdr)) {
		fprintf(stderr, "%s: fread: %s", argv0, strerror(errno));
		exit(1);
	}

	if (memcmp("farbfeld", hdr, sizeof("farbfeld") - 1)) {
		fprintf(stderr, "%s: invalid magic value\n", argv0);
		exit(1);
	}

	*width = ntohl(hdr[2]);
	*height = ntohl(hdr[3]);
}

void
write_ff_header(uint32_t width, uint32_t height)
{
	uint32_t tmp;

	fputs("farbfeld", stdout);

	tmp = htonl(width);
	if (fwrite(&tmp, sizeof(tmp), 1, stdout) != 1) {
		fprintf(stderr, "%s: write: %s", argv0, strerror(errno));
		exit(1);
	}

	tmp = htonl(height);
	if (fwrite(&tmp, sizeof(tmp), 1, stdout) != 1) {
		fprintf(stderr, "%s: write: %s", argv0, strerror(errno));
		exit(1);
	}
}
