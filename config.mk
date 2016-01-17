# farbfeld version
VERSION = 1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

PNGLIB = /usr/local/lib
PNGINC = /usr/local/include

JPGLIB = /usr/local/lib
JPGINC = /usr/local/include

LCMSLIB = /usr/local/lib
LCMSINC = /usr/local/include

INCS = -I${PNGINC} -I${JPGINC} -I${LCMSINC}
LIBS = -L${PNGLIB} -L${JPGLIB} -L${LCMSLIB} -lpng -ljpeg -llcms2

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

# compiler and linker
CC = cc
