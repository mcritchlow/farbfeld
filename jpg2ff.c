/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <jpeglib.h>

static jmp_buf error_jump;

METHODDEF(void)
jpeg_error(j_common_ptr cinfo)
{
	(*cinfo->err->output_message)(cinfo);
	longjmp(error_jump, 1);
}

int
main(int argc, char *argv[])
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	uint32_t width, height, val_be;
	uint16_t *ff_row = NULL;
	size_t jpeg_row_len, ff_row_len, i, dx, sx;
	int ret = 1;
	JSAMPARRAY buffer; /* output row buffer */

	if (argc > 1) {
		fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}

	/* load jpg */
	cinfo.err = jpeg_std_error(&jerr);

	jerr.error_exit = jpeg_error;
	if (setjmp(error_jump)) {
		goto cleanup;
	}

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
	ff_row_len = strlen("RGBA") * sizeof(uint16_t) * width;
	if(!(ff_row = malloc(ff_row_len))) {
		fprintf(stderr, "%s: malloc: out of memory\n", argv[0]);
		return 1;
	}

	/* write header */
	fprintf(stdout, "farbfeld");
	val_be = htonl(width);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);
	val_be = htonl(height);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);

	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient. */
		jpeg_read_scanlines(&cinfo, buffer, 1);

		for(i = 0, dx = 0, sx = 0; i < width; i++, sx += 3, dx += 4) {
			ff_row[dx]   = htons(buffer[0][sx]   * 257);
			ff_row[dx+1] = htons(buffer[0][sx+1] * 257);
			ff_row[dx+2] = htons(buffer[0][sx+2] * 257);
			ff_row[dx+3] = htons(65535);
		}

		/* write data */
		if (fwrite(ff_row, 1, ff_row_len, stdout) != ff_row_len) {
			fprintf(stderr, "%s: fwrite: ");
			perror(NULL);
			goto cleanup;
		}
	}
	jpeg_finish_decompress(&cinfo);
	ret = 0;

	/* flush output */
        if (fflush(stdout)) {
                fprintf(stderr, "%s: fflush stdout: ", argv[0]);
                perror(NULL);
                ret = 1;
        }
        if (fclose(stdout) && !ret) {
                fprintf(stderr, "%s: fclose stdout: ", argv[0]);
                perror(NULL);
                ret = 1;
        }
cleanup:
	free(ff_row);
	jpeg_destroy_decompress(&cinfo);

	return ret;
}
