/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lcms2.h>
#include <png.h>

static char *argv0;

/* ProPhoto RGB */
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
	cmsHPROFILE in_prof, out_prof;
	cmsHTRANSFORM trans;
	cmsToneCurve *gamma18, *out_curves[3];
	png_structp pngs;
	png_infop pngi;
	int icc_compression;
	uint32_t width, height, icc_len, outrowlen, tmp32, r, i;
	uint16_t *inrow, *outrow;
	uint8_t **png_row_p, *icc_data;
	char *icc_name;

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

	/* icc profile (output ProPhoto RGB) */
	if (png_get_valid(pngs, pngi, PNG_INFO_iCCP)) {
		png_get_iCCP(pngs, pngi, &icc_name,
		             &icc_compression, &icc_data, &icc_len);
		if (!(in_prof = cmsOpenProfileFromMem(icc_data,
		                                         icc_len)))
			goto lcmserr;
	} else {
		if (!(in_prof = cmsCreate_sRGBProfile()))
			goto lcmserr;
	}
	if (!(gamma18 = cmsBuildGamma(NULL, 1.8)))
		goto lcmserr;
	out_curves[0] = out_curves[1] = out_curves[2] = gamma18;
	if (!(out_prof = cmsCreateRGBProfile(cmsD50_xyY(), &primaries,
	                                        out_curves)))
		goto lcmserr;

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
		if (!(trans = cmsCreateTransform(in_prof, TYPE_RGBA_8,
		                                 out_prof, TYPE_RGBA_16,
		                                 INTENT_RELATIVE_COLORIMETRIC,
		                                 0)))
			goto lcmserr;
		for (r = 0; r < height; ++r) {
			cmsDoTransform(trans, png_row_p[r], outrow, width);
			for (i = 0; i < outrowlen; i++) {
				/* re-add alpha */
				if (i >= 3 && (i - 3) % 4 == 0)
					outrow[i] = 257 * png_row_p[r][i];
				/* swap endiannes to BE */
				outrow[i] = htons(outrow[i]);
			}
			if (fwrite(outrow, sizeof(uint16_t), outrowlen,
			           stdout) != outrowlen)
				goto writerr;
		}
		break;
	case 16:
		if (!(trans = cmsCreateTransform(in_prof, TYPE_RGBA_16,
		                                 out_prof, TYPE_RGBA_16,
		                                 INTENT_RELATIVE_COLORIMETRIC,
		                                 0)))
			goto lcmserr;
		for (r = 0; r < height; ++r) {
			inrow = (uint16_t *)png_row_p[r];
			for (i = 0; i < outrowlen; i++) {
				/* swap endianness to LE */
				inrow[i] = ntohs(inrow[i]);
			}
			cmsDoTransform(trans, png_row_p[r], outrow, width);
			for (i = 0; i < outrowlen; ++i) {
				/* re-add alpha */
				if (i >= 3 && (i - 3) % 4 == 0)
					outrow[i] = inrow[i];
				/* swap endianness to BE */
				outrow[i] = htons(outrow[i]);
			}
			if (fwrite(outrow, sizeof(uint16_t), outrowlen,
			           stdout) != outrowlen)
				goto writerr;
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
lcmserr:
	fprintf(stderr, "%s: lcms error\n", argv0);

	return 1;
}
