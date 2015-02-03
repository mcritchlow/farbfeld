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
	@${CC} -o $@ gif2if.o ${GIF_LIBS} ${LDFLAGS}

jpg2if: jpg2if.o
	@echo CC -o $@
	@${CC} -o $@ jpg2if.o ${JPEG_LIBS} ${LDFLAGS}

png2if: png2if.o
	@echo CC -o $@
	@${CC} -o $@ png2if.o ${PNG_LIBS} ${LDFLAGS}

if2png: if2png.o
	@echo CC -o $@
	@${CC} -o $@ if2png.o ${PNG_LIBS} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f png2if if2png jpg2if gif2if ${OBJ}

install: all
	@echo installing executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f png2if jpg2if if2png gif2if ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/png2if
	@chmod 755 ${DESTDIR}${PREFIX}/bin/jpg2if
	@chmod 755 ${DESTDIR}${PREFIX}/bin/if2png
	@chmod 755 ${DESTDIR}${PREFIX}/bin/gif2if

uninstall:
	@echo removing executable files from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/png2if
	@rm -f ${DESTDIR}${PREFIX}/bin/jpg2if
	@rm -f ${DESTDIR}${PREFIX}/bin/if2png
	@rm -f ${DESTDIR}${PREFIX}/bin/gif2if

.PHONY: all options clean install uninstall
