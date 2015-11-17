# Customize below to fit your system

# paths
PREFIX = /usr/local

PNGLIB = /usr/local/lib
PNGINC = /usr/local/include

INCS = -I${PNGINC}
LIBS = -L${PNGLIB} -lpng

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

# compiler and linker
CC = cc
