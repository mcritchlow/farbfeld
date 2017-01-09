/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char *argv0;

int
main(int argc, char *argv[])
{
	uint32_t hdr[4], width, height;
	char buf[BUFSIZ];
	size_t n, t;

	argv0 = argv[0], argc--, argv++;

	if (argc) {
		fprintf(stderr, "usage: %s\n", argv0);
		return 1;
	}

	/* header */
	if (fread(hdr, sizeof(*hdr), 4, stdin) != 4) {
		fprintf(stderr, "%s: file too short\n", argv0);
		return 1;
	}
	if (memcmp("farbfeld", hdr, sizeof("farbfeld") - 1)) {
		fprintf(stderr, "%s: invalid magic value\n", argv0);
		return 1;
	}
	width = ntohl(hdr[2]);
	height = ntohl(hdr[3]);

	/* write header */
	printf("P7\n"
	       "WIDTH %" PRIu32 "\n"
	       "HEIGHT %" PRIu32 "\n"
	       "DEPTH 4\n" /* number of channels */
	       "MAXVAL 65535\n"
	       "TUPLTYPE RGB_ALPHA\n"
	       "ENDHDR\n",
	       width, height);

	/* write image */
	t = (size_t)width * (size_t)height * sizeof(uint16_t) * (sizeof("RGBA") - 1);
	for (; (n = fread(buf, 1, sizeof(buf) <= t ? sizeof(buf) : t, stdin)); ) {
		t -= n;
		fwrite(buf, 1, n, stdout);

		if (feof(stdin)) {
			break;
		}
		if (ferror(stdin)) {
			fprintf(stderr, "%s: read: %s\n", argv0, strerror(errno));
			return 1;
		}
		if (ferror(stdout)) {
			fprintf(stderr, "%s: write: %s\n", argv0, strerror(errno));
			return 1;
		}
	}

	if (t > 0) {
		fprintf(stderr, "%s: file too short\n", argv0);
		return 1;
	}

	return 0;
}
