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
ff_read_header(uint32_t *width, uint32_t *height)
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
ff_write_header(uint32_t width, uint32_t height)
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

int
parse_mask(const char *s, uint16_t mask[3])
{
	size_t slen, i;
	unsigned int col[3], colfac;
	char fmt[] = "%#x%#x%#x";

	slen = strlen(s);
	if (slen != 3 && slen != 6 && slen != 12) {
		return 1;
	}

	fmt[1] = fmt[4] = fmt[7] = ((slen / 3) + '0');
	if (sscanf(s, fmt, col, col + 1, col + 2) != 3) {
		return 1;
	}

	colfac = (slen == 3) ? UINT16_MAX / 0xf :
	         (slen == 6) ? UINT16_MAX / 0xff :
	                       UINT16_MAX / 0xffff;

	for (i = 0; i < 3; i++) {
		mask[i] = col[i] * colfac;
	}

	return 0;
}

int
fshut(FILE *fp, const char *fname)
{
	int ret = 0;

	/* fflush() is undefined for input streams by ISO C,
	 * but not POSIX 2008 if you ignore ISO C overrides.
	 * Leave it unchecked and rely on the following
	 * functions to detect errors.
	 */
	fflush(fp);

	if (ferror(fp) && !ret) {
		fprintf(stderr, "%s: ferror %s: %s\n", argv0, fname,
		        strerror(errno));
		ret = 1;
	}

	if (fclose(fp) && !ret) {
		fprintf(stderr, "%s: fclose %s: %s\n", argv0, fname,
		        strerror(errno));
		ret = 1;
	}

	return ret;
}

void *
ereallocarray(void *optr, size_t nmemb, size_t size)
{
	void *p;

	if (!(p = reallocarray(optr, nmemb, size))) {
		fprintf(stderr, "%s: reallocarray: out of memory\n", argv0);
		exit(1);
	}

	return p;
}

long long
estrtonum(const char *numstr, long long minval, long long maxval)
{
	const char *errstr;
	long long ll;

	ll = strtonum(numstr, minval, maxval, &errstr);
	if (errstr) {
		fprintf(stderr, "%s: strtonum %s: %s\n", argv0, numstr, errstr);
		exit(1);
	}

	return ll;
}

/*
 * Copyright (c) 2008 Otto Moerbeek <otto@drijf.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW	(1UL << (sizeof(size_t) * 4))

void *
reallocarray(void *optr, size_t nmemb, size_t size)
{
	if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
	    nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(optr, size * nmemb);
}

/*	$OpenBSD: strtonum.c,v 1.7 2013/04/17 18:40:58 tedu Exp $	*/

/*
 * Copyright (c) 2004 Ted Unangst and Todd Miller
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#define	INVALID		1
#define	TOOSMALL	2
#define	TOOLARGE	3

long long
strtonum(const char *numstr, long long minval, long long maxval,
         const char **errstrp)
{
	long long ll = 0;
	int error = 0;
	char *ep;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = errno;
	errno = 0;
	if (minval > maxval) {
		error = INVALID;
	} else {
		ll = strtoll(numstr, &ep, 10);
		if (numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
			error = TOOSMALL;
		else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
			error = TOOLARGE;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		ll = 0;

	return (ll);
}
