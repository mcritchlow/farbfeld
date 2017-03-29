# farbfeld version
VERSION = 2

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

# flags
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS   = -std=c89 -pedantic -Wall -Os
LDFLAGS  = -s

png2ff ff2png: LDFLAGS += -lpng
jpg2ff ff2jpg: LDFLAGS += -ljpeg

# compiler and linker
CC = cc
