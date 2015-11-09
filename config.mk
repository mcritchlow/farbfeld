# Customize below to fit your system

# paths
PREFIX = /usr/local

# libs
LIBS =
PNG_LIBS = -lpng
JPEG_LIBS = -ljpeg
GIF_LIBS = -lgif

# flags
CPPFLAGS =

# debug
#CFLAGS = -std=c99 -pedantic -Wall -Wextra -O0 -g -ggdb ${CPPFLAGS}
#LDFLAGS = ${LIBS}
# optimized
CFLAGS = -std=c99 -pedantic -Wall -Wextra -D_DEFAULT_SOURCE -Os ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
