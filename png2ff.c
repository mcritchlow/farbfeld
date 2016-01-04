/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <png.h>

int
main(int argc, char *argv[])
{
	png_structp png_struct_p;
	png_infop png_info_p;
	png_bytepp png_row_p;
	int depth, color;
	uint32_t width, height, png_row_len, tmp32, r, i;
	uint16_t tmp16;

	if (argc > 1) {
		fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}

	/* load png */
	png_struct_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
	                                      NULL, NULL);
	png_info_p = png_create_info_struct(png_struct_p);

	if (!png_struct_p || !png_info_p) {
		fprintf(stderr, "failed to initialize libpng\n");
		return 1;
	}
	if (setjmp(png_jmpbuf(png_struct_p)))
		return 1;
	png_init_io(png_struct_p, stdin);
	if (png_get_valid(png_struct_p, png_info_p, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_struct_p);
	png_set_add_alpha(png_struct_p, 255*257, PNG_FILLER_AFTER);
	png_set_expand_gray_1_2_4_to_8(png_struct_p);
	png_set_gray_to_rgb(png_struct_p);
	png_set_packing(png_struct_p);
	png_read_png(png_struct_p, png_info_p, PNG_TRANSFORM_PACKING |
	             PNG_TRANSFORM_EXPAND, NULL);
	png_get_IHDR(png_struct_p, png_info_p, &width, &height, &depth,
	             &color, NULL, NULL, NULL);
	png_row_len = png_get_rowbytes(png_struct_p, png_info_p);
	png_row_p = png_get_rows(png_struct_p, png_info_p);

	/* write header */
	fprintf(stdout, "farbfeld");
	tmp32 = htonl(width);
	fwrite(&tmp32, sizeof(uint32_t), 1, stdout);
	tmp32 = htonl(height);
	fwrite(&tmp32, sizeof(uint32_t), 1, stdout);

	/* write data */
	switch(depth) {
	case 8:
		for (r = 0; r < height; ++r) {
			for (i = 0; i < png_row_len; i++) {
				/* ((2^16-1) / 255) == 257 */
				tmp16 = htons(257 * png_row_p[r][i]);
				fwrite(&tmp16, sizeof(uint16_t), 1, stdout);
			}
		}
		break;
	case 16:
		for (r = 0; r < height; ++r) {
			for (i = 0; i < png_row_len / 2; i++) {
				tmp16 = *((uint16_t *)
				                (png_row_p[r] + 2 * i));
				fwrite(&tmp16, sizeof(uint16_t), 1, stdout);
			}
		}
		break;
	default:
		fprintf(stderr, "format error\n");
		return 1;
	}

	png_destroy_read_struct(&png_struct_p, &png_info_p, NULL);

	return 0;
}
