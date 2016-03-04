/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>

static char *argv0;

METHODDEF(void)
jpeg_error(j_common_ptr js)
{
	fprintf(stderr, "%s: libjpeg: ", argv0);
	(*js->err->output_message)(js);
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct jpeg_decompress_struct js;
	struct jpeg_error_mgr jerr;
	uint32_t width, height, tmp32;
	uint16_t *row;
	size_t rowlen, i;
	JSAMPARRAY jpgrow;

	argv0 = argv[0], argc--, argv++;

	if (argc) {
		fprintf(stderr, "usage: %s\n", argv0);
		return 1;
	}

	/* load jpg */
	js.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_error;

	jpeg_create_decompress(&js);

	jpeg_stdio_src(&js, stdin);

	jpeg_read_header(&js, 1);
	width = js.image_width;
	height = js.image_height;

	/* set output format */
	js.output_components = 3;     /* color components per pixel */
	js.out_color_space = JCS_RGB; /* input color space */

	jpeg_start_decompress(&js);

	/* create output buffers */
	jpgrow = (*js.mem->alloc_sarray)((j_common_ptr)&js,
	                                 JPOOL_IMAGE, width *
	                                 js.output_components, 1);
	rowlen = strlen("RGBA") * width;
	if(!(row = malloc(rowlen * sizeof(uint16_t)))) {
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

	while (js.output_scanline < js.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to
		 * scanlines.
		 * Here the array is only one element long, but you could
		 * ask for more than one scanline at a time if that's more
		 * convenient. */
		jpeg_read_scanlines(&js, jpgrow, 1);

		for (i = 0; i < width; ++i) {
			row[4*i + 0] = htons(jpgrow[0][3*i + 0] * 257);
			row[4*i + 1] = htons(jpgrow[0][3*i + 1] * 257);
			row[4*i + 2] = htons(jpgrow[0][3*i + 2] * 257);
			row[4*i + 3] = htons(65535);
		}

		/* write data */
		if (fwrite(row, 2, rowlen, stdout) != rowlen)
			goto writerr;
	}
	jpeg_finish_decompress(&js);
	jpeg_destroy_decompress(&js);

	return 0;
writerr:
	fprintf(stderr, "%s: fwrite: ", argv0);
	perror(NULL);

	return 1;
}
