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
pngerr(png_structp png_struct_p, png_const_charp msg)
{
	fprintf(stderr, "%s: libpng: %s\n", argv0, msg);
	exit(1);
}

int
main(int argc, char *argv[])
{
	cmsHPROFILE in_profile, out_profile;
	cmsHTRANSFORM transform;
	cmsToneCurve *gamma18, *out_curves[3];
	png_structp png_struct_p;
	png_infop png_info_p;
	png_bytep *png_row_p, icc_data;
	png_charp icc_name;
	uint32_t width, height, icc_len, outrowlen, tmp32, r, i;
	uint16_t *inrow, *outrow;
	int icc_compression;

	argv0 = argv[0], argc--, argv++;

	if (argc) {
		fprintf(stderr, "usage: %s\n", argv0);
		return 1;
	}

	/* load png */
	png_struct_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
	                                      pngerr, NULL);
	png_info_p = png_create_info_struct(png_struct_p);

	if (!png_struct_p || !png_info_p) {
		fprintf(stderr, "%s: failed to initialize libpng\n", argv0);
		return 1;
	}
	png_init_io(png_struct_p, stdin);
	if (png_get_valid(png_struct_p, png_info_p, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_struct_p);
	png_set_add_alpha(png_struct_p, 255*257, PNG_FILLER_AFTER);
	png_set_expand_gray_1_2_4_to_8(png_struct_p);
	png_set_gray_to_rgb(png_struct_p);
	png_set_packing(png_struct_p);
	png_read_png(png_struct_p, png_info_p, PNG_TRANSFORM_PACKING |
	             PNG_TRANSFORM_EXPAND, NULL);
	width = png_get_image_width(png_struct_p, png_info_p);
	height = png_get_image_height(png_struct_p, png_info_p);
	png_row_p = png_get_rows(png_struct_p, png_info_p);

	/* icc profile (output ProPhoto RGB) */
	if (png_get_valid(png_struct_p, png_info_p, PNG_INFO_iCCP)) {
		png_get_iCCP(png_struct_p, png_info_p, &icc_name,
		             &icc_compression, &icc_data, &icc_len);
		if (!(in_profile = cmsOpenProfileFromMem(icc_data,
		                                         icc_len)))
			goto lcmserr;
	} else {
		if (!(in_profile = cmsCreate_sRGBProfile()))
			goto lcmserr;
	}
	if (!(gamma18 = cmsBuildGamma(NULL, 1.8)))
		goto lcmserr;
	out_curves[0] = out_curves[1] = out_curves[2] = gamma18;
	if (!(out_profile = cmsCreateRGBProfile(cmsD50_xyY(), &primaries,
	                                        out_curves)))
		goto lcmserr;

	/* allocate row buffer */
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
	switch(png_get_bit_depth(png_struct_p, png_info_p)) {
	case 8:
		if (!(transform = cmsCreateTransform(in_profile,
		                  TYPE_RGBA_8, out_profile, TYPE_RGBA_16,
		                  INTENT_RELATIVE_COLORIMETRIC, 0)))
			goto lcmserr;
		for (r = 0; r < height; ++r) {
			cmsDoTransform(transform, png_row_p[r], outrow, width);
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
		if (!(transform = cmsCreateTransform(in_profile,
		                  TYPE_RGBA_16, out_profile, TYPE_RGBA_16,
		                  INTENT_RELATIVE_COLORIMETRIC, 0)))
			goto lcmserr;
		for (r = 0; r < height; ++r) {
			inrow = (uint16_t *)png_row_p[r];
			for (i = 0; i < outrowlen; i++) {
				/* swap endianness to LE */
				inrow[i] = ntohs(inrow[i]);
			}
			cmsDoTransform(transform, png_row_p[r], outrow, width);
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

	png_destroy_read_struct(&png_struct_p, &png_info_p, NULL);

	return 0;
writerr:
	fprintf(stderr, "%s: fwrite: ", argv0);
	perror(NULL);

	return 1;
lcmserr:
	fprintf(stderr, "%s: lcms error\n", argv0);

	return 1;
}
