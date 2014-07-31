/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <setjmp.h>
#include <jpeglib.h>
#include "arg.h"

char *argv0;

static jmp_buf setjmp_buffer;

static void
usage(void)
{
	fprintf(stderr, "usage: %s\n", argv0);
	exit(EXIT_FAILURE);
}

METHODDEF(void)
if_jpeg_error(j_common_ptr cinfo)
{
	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(setjmp_buffer, 1);
}

int
main(int argc, char *argv[])
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	uint32_t width, height, val_be;
	uint8_t *if_row = NULL;
	size_t jpeg_row_len, if_row_len, i, dx, sx;
	int status = EXIT_FAILURE;
	JSAMPARRAY buffer; /* output row buffer */

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc != 0)
		usage();

	/* load jpeg */
	cinfo.err = jpeg_std_error(&jerr);

	jerr.error_exit = if_jpeg_error;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return. */
		goto cleanup;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, stdin);

	jpeg_read_header(&cinfo, TRUE);
	width = cinfo.image_width;
	height = cinfo.image_height;

	/* change output for imagefile */
	cinfo.output_components = 3;     /* # of color components per pixel */
	cinfo.out_color_space = JCS_RGB; /* colorspace of input image */

	jpeg_start_decompress(&cinfo);
	jpeg_row_len = width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
	         ((j_common_ptr) &cinfo, JPOOL_IMAGE, jpeg_row_len, 1);
	if_row_len = strlen("RGBA") * width;
	if(!(if_row = malloc(if_row_len))) {
		fprintf(stderr, "Can't malloc\n");
		return EXIT_FAILURE;
	}

	/* write header with big endian width and height-values */
	fprintf(stdout, "imagefile");
	val_be = htonl(width);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);
	val_be = htonl(height);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);

	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient. */
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);

		for(i = 0, dx = 0, sx = 0; i < width; i++, sx += 3, dx += 4) {
			if_row[dx] = buffer[0][sx];
			if_row[dx+1] = buffer[0][sx+1];
			if_row[dx+2] = buffer[0][sx+2];
			if_row[dx+3] = 255;
		}
		/* write data */
		if (fwrite(if_row, 1, if_row_len, stdout) != if_row_len) {
			fprintf(stderr, "fwrite() failed\n");
			goto cleanup;
		}
	}
	jpeg_finish_decompress(&cinfo);
	status = EXIT_SUCCESS;

cleanup:
	free(if_row);
	jpeg_destroy_decompress(&cinfo);

	return status;
}
