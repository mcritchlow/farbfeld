/* See LICENSE file for copyright and license details.
 * code borrowed from util/gif2rgb.c from giflib-5.0 */
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gif_lib.h>
#include "arg.h"

char *argv0;

static void
usage(void)
{
	fprintf(stderr, "usage: %s\n", argv0);
	exit(EXIT_FAILURE);
}

static void
die(const char *s) {
	fputs(s, stderr);
	exit(EXIT_FAILURE);
}

void
gif_print_error(int code)
{
	const char *err = GifErrorString(code);

	if(err != NULL)
		fprintf(stderr, "GIF-LIB error: %s.\n", err);
	else
		fprintf(stderr, "GIF-LIB undefined error %d.\n", code);
	exit(EXIT_FAILURE);
}

void *
emalloc(size_t size) {
	void *p;

	if(!(p = malloc(size)))
		die("can't malloc\n");
	return p;
}

int
main(int argc, char *argv[])
{
	GifRecordType recordtype;
	GifFileType *giffile;
	GifRowType * gifrows;
	GifByteType *extension;
	GifColorType *colormapentry;
	ColorMapObject *colormap;
	uint32_t width, height, rwidth, rheight, val_be;
	uint32_t i, j, k;
	size_t gif_row_len;
	uint8_t *if_row;
	int errorcode, extcode, row, col, imagenum = 0;
	int interlacedoffset[] = { 0, 4, 2, 1 };
	int interlacedjumps[] = { 8, 8, 4, 2 };

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc != 0)
		usage();

	/* load gif */
	if ((giffile = DGifOpenFileHandle(0, &errorcode)) == NULL)
		gif_print_error(errorcode);

	width = (uint32_t)giffile->SWidth;
	height = (uint32_t)giffile->SHeight;
	gif_row_len = giffile->SWidth * sizeof(GifPixelType);
	gifrows = emalloc(giffile->SHeight * sizeof(GifRowType));
	gifrows[0] = emalloc(gif_row_len);

	/* set color to BackGround. */
	for (i = 0; i < width; i++)
		gifrows[0][i] = giffile->SBackGroundColor;
	for(i = 1; i < height; i++) {
		gifrows[i] = emalloc(gif_row_len);
		memcpy(gifrows[i], gifrows[0], gif_row_len);
	}

	/* scan the content of the GIF file and load the image(s) in: */
	do {
		if (DGifGetRecordType(giffile, &recordtype) == GIF_ERROR)
			gif_print_error(giffile->Error);

		switch (recordtype) {
		case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(giffile) == GIF_ERROR)
				gif_print_error(giffile->Error);

			/* image position relative to Screen. */
			row = giffile->Image.Top;
			col = giffile->Image.Left;
			rwidth = giffile->Image.Width;
			rheight = giffile->Image.Height;
			if (giffile->Image.Left + giffile->Image.Width > giffile->SWidth ||
				giffile->Image.Top + giffile->Image.Height > giffile->SHeight) {
				fprintf(stderr, "Image %d is not confined to screen dimension, aborted.\n", imagenum);
				exit(EXIT_FAILURE);
			}
			if (giffile->Image.Interlace) {
				/* need to perform 4 passes on the images: */
				for (i = 0; i < 4; i++) {
					for (j = row + interlacedoffset[i]; j < row + rheight; j += interlacedjumps[i]) {
						if (DGifGetLine(giffile, &gifrows[j][col], rwidth) == GIF_ERROR)
							gif_print_error(giffile->Error);
					}
				}
			} else {
				for (i = 0; i < rheight; i++) {
					if (DGifGetLine(giffile, &gifrows[row++][col], rwidth) == GIF_ERROR)
						gif_print_error(giffile->Error);
				}
			}
			/* this is set to disallow multiple images */
			recordtype = TERMINATE_RECORD_TYPE;
			imagenum++;
			break;
		case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(giffile, &extcode, &extension) == GIF_ERROR)
				gif_print_error(giffile->Error);
			while (extension != NULL) {
				if (DGifGetExtensionNext(giffile, &extension) == GIF_ERROR)
					gif_print_error(giffile->Error);
			}
			break;
		case TERMINATE_RECORD_TYPE:
			 break;
		default:
			break;
		}
	} while (recordtype != TERMINATE_RECORD_TYPE);

	if(giffile->Image.ColorMap)
		colormap = giffile->Image.ColorMap;
	else
		colormap = giffile->SColorMap;

	if(colormap == NULL)
		die("Gif Image does not have a colormap\n");

	 /* write header with big endian width and height-values */
	fprintf(stdout, "imagefile");
	val_be = htonl(width);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);
	val_be = htonl(height);
	fwrite(&val_be, sizeof(uint32_t), 1, stdout);

	gif_row_len = width * strlen("RGBA");
	if_row = emalloc(gif_row_len);

	/* write data */
	for (i = 0; i < height; i++) {
		for(j = 0, k = 0; j < width; j++, k += 4) {
			colormapentry = &colormap->Colors[gifrows[i][j]];
			if_row[k] = colormapentry->Red;
			if_row[k+1] = colormapentry->Green;
			if_row[k+2] = colormapentry->Blue;
			if_row[k+3] = 255; /* TODO: mask? */
		}
		if (fwrite(if_row, 1, gif_row_len, stdout) != gif_row_len)
			die("fwrite() failed\n");
	}

	/* cleanup */
	for(i = 0; i < height; i++)
		free(gifrows[i]);
	free(gifrows);
	free(if_row);

	return EXIT_SUCCESS;
}
