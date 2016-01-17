/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>
#include <lcms2.h>

static char *argv0;

/* ProPhoto RGB */
static cmsCIExyYTRIPLE primaries = {
	/*     x,      y,        Y */
	{ 0.7347, 0.2653, 0.288040 }, /* red   */
	{ 0.1596, 0.8404, 0.711874 }, /* green */
	{ 0.0366, 0.0001, 0.000086 }, /* blue  */
};


METHODDEF(void)
jpeg_error(j_common_ptr cinfo)
{
	fprintf(stderr, "%s: libjpeg: ", argv0);
	(*cinfo->err->output_message)(cinfo);
	exit(1);
}

int
main(int argc, char *argv[])
{
	cmsHPROFILE in_profile, out_profile;
	cmsHTRANSFORM transform;
	cmsToneCurve *gamma18, *out_curves[3];
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	uint32_t width, height, val_be;
	uint16_t *ff_row;
	size_t jpeg_row_len, ff_row_len, i, dx, sx;
	JSAMPARRAY buffer; /* output row buffer */

	argv0 = argv[0], argc--, argv++;

	if (argc > 1) {
		fprintf(stderr, "usage: %s\n", argv0);
		return 1;
	}

	/* load jpg */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_error;

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, stdin);

	jpeg_read_header(&cinfo, TRUE);
	width = cinfo.image_width;
	height = cinfo.image_height;

	/* set output format */
	cinfo.output_components = 3;     /* color components per pixel */
	cinfo.out_color_space = JCS_RGB; /* input color space */

	jpeg_start_decompress(&cinfo);
	jpeg_row_len = width * cinfo.output_components;

	/* create output buffers */
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo,
	                                    JPOOL_IMAGE, jpeg_row_len, 1);
	ff_row_len = strlen("RGBA") * width;
	if(!(ff_row = malloc(ff_row_len * sizeof(uint16_t)))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv0);
		return 1;
	}

	/* icc profile (output ProPhoto RGB) */
	if (!(in_profile = cmsCreate_sRGBProfile()))
		goto lcmserr;
	if (!(gamma18 = cmsBuildGamma(NULL, 1.8)))
		goto lcmserr;
	out_curves[0] = out_curves[1] = out_curves[2] = gamma18;
	if (!(out_profile = cmsCreateRGBProfile(cmsD50_xyY(), &primaries,
	                                        out_curves)))
		goto lcmserr;
	if (!(transform = cmsCreateTransform(in_profile, TYPE_RGBA_16,
	                                     out_profile, TYPE_RGBA_16,
	                                     INTENT_RELATIVE_COLORIMETRIC, 0)))
		goto lcmserr;

	/* write header */
	fprintf(stdout, "farbfeld");
	val_be = htonl(width);
	if (fwrite(&val_be, sizeof(uint32_t), 1, stdout) != 1)
		goto writerr;
	val_be = htonl(height);
	if (fwrite(&val_be, sizeof(uint32_t), 1, stdout) != 1)
		goto writerr;

	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient. */
		jpeg_read_scanlines(&cinfo, buffer, 1);

		for (i = 0, dx = 0, sx = 0; i < width; i++, sx += 3, dx += 4) {
			ff_row[dx]   = buffer[0][sx]   * 257;
			ff_row[dx+1] = buffer[0][sx+1] * 257;
			ff_row[dx+2] = buffer[0][sx+2] * 257;
			ff_row[dx+3] = 65535;
		}

		cmsDoTransform(transform, ff_row, ff_row, ff_row_len);

		for (i = 0; i < ff_row_len; i++) {
			/* re-add alpha */
			if (i >= 3 && (i - 3) % 4 == 0)
				ff_row[i] = 65535;
			/* swap endianness to BE */
			ff_row[i] = htons(ff_row[i]);
		}

		/* write data */
		if (fwrite(ff_row, 2, ff_row_len, stdout) != ff_row_len)
			goto writerr;
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;
writerr:
	fprintf(stderr, "%s: fwrite: ", argv0);
	perror(NULL);

	return 1;
lcmserr:
	fprintf(stderr, "%s: lcms error\n", argv0);

	return 1;
}
