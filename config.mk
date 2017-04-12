# farbfeld version
VERSION = 2

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Os
LDFLAGS  = -s

png2ff-LDFLAGS = -lpng
ff2png-LDFLAGS = -lpng
jpg2ff-LDFLAGS = -ljpeg
ff2jpg-LDFLAGS = -ljpeg

# compiler and linker
CC = cc
