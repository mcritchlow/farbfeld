# farbfeld version
VERSION = 2

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

PNGLIB = /usr/local/lib
PNGINC = /usr/local/include

JPGLIB = /usr/local/lib
JPGINC = /usr/local/include

INCS =
LIBS =

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS  = -s ${LIBS}

# compiler and linker
CC = cc

# flags per tool.

png2ff-CFLAGS := -I${PNGINC}
png2ff-LDFLAGS := -L${PNGLIB} -lpng

ff2png-CFLAGS := -I${PNGINC}
ff2png-LDFLAGS := -L${PNGLIB} -lpng

jpg2ff-CFLAGS := -I${JPGINC}
jpg2ff-LDFLAGS := -L${JPGLIB} -ljpeg

ff2jpg-CFLAGS := -I${JPGINC}
ff2jpg-LDFLAGS := -L${JPGLIB} -ljpeg
