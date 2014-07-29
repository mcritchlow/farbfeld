# imagefile - tools to convert between png and if
# See LICENSE file for copyright and license details

include config.mk

SRC = png2if.c if2png.c jpg2if.c gif2if.c
OBJ = ${SRC:.c=.o}

all: options png2if if2png jpg2if gif2if

options:
	@echo imagefile build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c -o $@ ${CFLAGS} $<

${OBJ}: config.mk

gif2if: gif2if.o
	@echo CC -o $@
	@${CC} -o $@ gif2if.o ${LDFLAGS} ${GIF_LIBS}

jpg2if: jpg2if.o
	@echo CC -o $@
	@${CC} -o $@ jpg2if.o ${LDFLAGS} ${JPEG_LIBS}

png2if: png2if.o
	@echo CC -o $@
	@${CC} -o $@ png2if.o ${LDFLAGS} ${PNG_LIBS}

if2png: if2png.o
	@echo CC -o $@
	@${CC} -o $@ if2png.o ${LDFLAGS} ${PNG_LIBS}

clean:
	@echo cleaning
	@rm -f png2if if2png jpg2if gif2if ${OBJ}

install: all
	@echo installing executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f png2if if2png jpg2if ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/png2if
	@chmod 755 ${DESTDIR}${PREFIX}/bin/jpg2if
	@chmod 755 ${DESTDIR}${PREFIX}/bin/if2png

uninstall:
	@echo removing executable files from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/png2if
	@rm -f ${DESTDIR}${PREFIX}/bin/if2png

.PHONY: all options clean install uninstall
