# Customize below to fit your system

# paths
PREFIX = /usr/local

# libs
LIBS = -lpng

# flags
CFLAGS = -std=c90 -pedantic -ansi -Wall -Wextra -Wno-pointer-sign -Wno-maybe-uninitialized -Os
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
