/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lcms2.h>
#include <png.h>

#define HEADER "farbfeld########"

static char *argv0;

/* ROMM RGB primaries (ISO 22028-2:2013) */
static cmsCIExyYTRIPLE primaries = {
	/*     x,      y,        Y */
	{ 0.7347, 0.2653, 0.288040 }, /* red   */
	{ 0.1596, 0.8404, 0.711874 }, /* green */
	{ 0.0366, 0.0001, 0.000086 }, /* blue  */
};

void
pngerr(png_structp pngs, const char *msg)
{
	fprintf(stderr, "%s: libpng: %s\n", argv0, msg);
	exit(1);
}

int
main(int argc, char *argv[])
{
	cmsContext icc_context;
	cmsHPROFILE out_prof;
	cmsMLU *mlu1, *mlu2, *mlu3;
	cmsToneCurve *gamma10, *out_curve[3];
	png_structp pngs;
	png_infop pngi;
	size_t png_row_len, j;
	uint32_t icclen, width, height, i;
	uint16_t tmp16, *png_row;
	uint8_t hdr[16], *icc;

	argv0 = argv[0], argc--, argv++;

	if (argc) {
		fprintf(stderr, "usage: %s\n", argv0);
		return 1;
	}

	/* header */
	if (fread(hdr, 1, strlen(HEADER), stdin) != strlen(HEADER)) {
		fprintf(stderr, "%s: incomplete header\n", argv0);
		return 1;
	}
	if (memcmp("farbfeld", hdr, strlen("farbfeld"))) {
		fprintf(stderr, "%s: invalid magic value\n", argv0);
		return 1;
	}
	width = ntohl(*((uint32_t *)(hdr + 8)));
	height = ntohl(*((uint32_t *)(hdr + 12)));

	/* icc profile (linear ROMM RGB (ISO 22028-2:2013)) */
	if (!(icc_context = cmsCreateContext(NULL, NULL)))
		goto lcmserr;
	if (!(gamma10 = cmsBuildGamma(icc_context, 1.0)))
		goto lcmserr;
	out_curve[0] = out_curve[1] = out_curve[2] = gamma10;
	if (!(out_prof = cmsCreateRGBProfileTHR(icc_context, cmsD50_xyY(),
	                                        &primaries, out_curve)))
		goto lcmserr;
	cmsSetHeaderFlags(out_prof, cmsEmbeddedProfileTrue | cmsUseAnywhere);
	cmsSetHeaderRenderingIntent(out_prof, INTENT_RELATIVE_COLORIMETRIC);
	cmsSetDeviceClass(out_prof, cmsSigColorSpaceClass);
	if (!(mlu1 = cmsMLUalloc(NULL, 1)) || !(mlu2 = cmsMLUalloc(NULL, 1)) ||
	    !(mlu3 = cmsMLUalloc(NULL, 1)))
		goto lcmserr;
	cmsMLUsetASCII(mlu1, "en", "US", "Public Domain");
	cmsWriteTag(out_prof, cmsSigCopyrightTag, mlu1);
	cmsMLUsetASCII(mlu2, "en", "US", "aka Linear ProPhoto RGB, Melissa RGB");
	cmsWriteTag(out_prof, cmsSigDeviceModelDescTag, mlu2);
	cmsMLUsetASCII(mlu3, "en", "US", "Linear ROMM RGB (ISO 22028-2:2013)");
	cmsWriteTag(out_prof, cmsSigProfileDescriptionTag, mlu3);
	cmsSaveProfileToMem(out_prof, NULL, &icclen);
	if (!(icc = malloc(icclen))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv0);
		return 1;
	}
	cmsSaveProfileToMem(out_prof, icc, &icclen);

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
	png_set_iCCP(pngs, pngi, "Linear ROMM RGB (ISO 22028-2:2013)", 0,
	             icc, icclen);
	png_write_info(pngs, pngi);

	/* write rows */
	png_row_len = strlen("RGBA") * width * sizeof(uint16_t);
	if (!(png_row = malloc(png_row_len))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv0);
		return 1;
	}
	for (i = 0; i < height; ++i) {
		for (j = 0; j < png_row_len / sizeof(uint16_t); ++j) {
			if (fread(&tmp16, sizeof(uint16_t), 1, stdin) != 1) {
				fprintf(stderr, "%s: unexpected EOF\n", argv0);
				return 1;
			}
			png_row[j] = tmp16;
		}
		png_write_row(pngs, (uint8_t *)png_row);
	}
	png_write_end(pngs, NULL);
	png_destroy_write_struct(&pngs, NULL);

	return 0;
lcmserr:
	fprintf(stderr, "%s: lcms error\n", argv0);

	return 1;
}
