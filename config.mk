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
#CFLAGS = -std=c90 -ansi -pedantic -Wall -Wextra -O0 -g -ggdb ${CPPFLAGS}
#LDFLAGS = ${LIBS}
# optimized
CFLAGS = -std=c90 -ansi -pedantic -Wall -Wextra -Os ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
