# Customize below to fit your system

# paths
PREFIX = /usr/local

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Os ${CPPFLAGS}
LDFLAGS  = -s -lpng -ljpeg -lgif

# compiler and linker
CC = cc
