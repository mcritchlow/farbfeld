# Customize below to fit your system

# paths
PREFIX = /usr/local

PNGLIB = /usr/local/lib
PNGINC = /usr/local/include

JPEGLIB = /usr/local/lib
JPEGINC = /usr/local/include

INCS = -I${PNGINC} -I${JPEGINC}
LIBS = -L${PNGLIB} -L${JPEGLIB} -lpng -ljpeg

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

# compiler and linker
CC = cc
