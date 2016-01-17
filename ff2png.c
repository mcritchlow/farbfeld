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

/* ProPhoto RGB */
static cmsCIExyYTRIPLE primaries = {
	/*     x,      y,        Y */
	{ 0.7347, 0.2653, 0.288040 }, /* red   */
	{ 0.1596, 0.8404, 0.711874 }, /* green */
	{ 0.0366, 0.0001, 0.000086 }, /* blue  */
};

void
pngerr(png_structp png_struct_p, png_const_charp msg)
{
	fprintf(stderr, "%s: libpng: %s\n", argv0, msg);
	exit(1);
}

int
main(int argc, char *argv[])
{
	cmsContext icc_context;
	cmsHPROFILE out_profile;
	cmsMLU *mlu1, *mlu2;
	cmsToneCurve *gamma18, *out_curves[3];
	png_structp png_struct_p;
	png_infop png_info_p;
	png_size_t png_row_len, j;
	png_uint_32 width, height, i;
	uint32_t icclen;
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

	/* icc profile (ProPhoto RGB) */
	if (!(icc_context = cmsCreateContext(NULL, NULL)))
		goto lcmserr;
	if (!(gamma18 = cmsBuildGamma(icc_context, 1.8)))
		goto lcmserr;
	out_curves[0] = out_curves[1] = out_curves[2] = gamma18;
	if (!(out_profile = cmsCreateRGBProfileTHR(icc_context, cmsD50_xyY(),
	                                        &primaries, out_curves)))
		goto lcmserr;
	cmsSetHeaderFlags(out_profile, cmsEmbeddedProfileTrue | cmsUseAnywhere);
	cmsSetHeaderRenderingIntent(out_profile, INTENT_RELATIVE_COLORIMETRIC);
	cmsSetDeviceClass(out_profile, cmsSigColorSpaceClass);
	if (!(mlu1 = cmsMLUalloc(NULL, 1)) || !(mlu2 = cmsMLUalloc(NULL, 1)))
		goto lcmserr;
	cmsMLUsetASCII(mlu1, "en", "US", "Public Domain");
	cmsWriteTag(out_profile, cmsSigCopyrightTag, mlu1);
	cmsMLUsetASCII(mlu2, "en", "US", "ProPhoto RGB");
	cmsWriteTag(out_profile, cmsSigProfileDescriptionTag, mlu2);
	cmsWriteTag(out_profile, cmsSigDeviceModelDescTag, mlu2);
	cmsSaveProfileToMem(out_profile, NULL, &icclen);
	if (!(icc = malloc(icclen))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv0);
		return 1;
	}
	cmsSaveProfileToMem(out_profile, icc, &icclen);

	/* load png */
	png_struct_p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
	                                       pngerr, NULL);
	png_info_p = png_create_info_struct(png_struct_p);

	if (!png_struct_p || !png_info_p) {
		fprintf(stderr, "%s: failed to initialize libpng\n", argv0);
		return 1;
	}
	png_init_io(png_struct_p, stdout);
	png_set_IHDR(png_struct_p, png_info_p, width, height, 16,
	             PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_set_iCCP(png_struct_p, png_info_p, "ProPhoto RGB", 0, icc, icclen);
	png_write_info(png_struct_p, png_info_p);

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
		png_write_row(png_struct_p, (uint8_t *)png_row);
	}
	png_write_end(png_struct_p, NULL);
	png_destroy_write_struct(&png_struct_p, NULL);

	return 0;
lcmserr:
	fprintf(stderr, "%s: lcms error\n", argv0);

	return 1;
}
