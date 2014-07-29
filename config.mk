# Customize below to fit your system

# paths
PREFIX = /usr/local

# libs
LIBS = -lpng

# flags
CFLAGS = -std=c90 -ansi -pedantic -Wall -Wextra
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
