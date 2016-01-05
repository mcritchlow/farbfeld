# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

PNGLIB = /usr/local/lib
PNGINC = /usr/local/include

JPGLIB = /usr/local/lib
JPGINC = /usr/local/include

INCS = -I${PNGINC} -I${JPGINC}
LIBS = -L${PNGLIB} -L${JPGLIB} -lpng -ljpeg

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

# compiler and linker
CC = cc
