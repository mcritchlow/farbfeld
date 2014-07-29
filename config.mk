# Customize below to fit your system

# paths
PREFIX = /usr/local

# libs
LIBS =
PNG_LIBS = -lpng
JPEG_LIBS = -ljpeg

# flags
CFLAGS = -std=c90 -ansi -pedantic -Wall -Wextra -Os
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
