/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stdio.h>

extern char *argv0;

#define LEN(x) (sizeof (x) / sizeof *(x))

void read_ff_header(uint32_t *width, uint32_t *height);
void write_ff_header(uint32_t width, uint32_t height);
