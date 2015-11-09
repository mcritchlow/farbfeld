# imagefile - tools to convert between png and ff
# See LICENSE file for copyright and license details

include config.mk

SRC = png2ff.c ff2png.c
OBJ = ${SRC:.c=.o}

all: options png2ff ff2png

options:
	@echo farbfeld build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c -o $@ ${CFLAGS} $<

${OBJ}: config.mk

png2ff: png2ff.o
	@echo CC -o $@
	@${CC} -o $@ png2ff.o ${PNG_LIBS} ${LDFLAGS}

ff2png: ff2png.o
	@echo CC -o $@
	@${CC} -o $@ ff2png.o ${PNG_LIBS} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f png2ff ff2png ${OBJ}

install: all
	@echo installing executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f png2ff ff2png ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/png2ff
	@chmod 755 ${DESTDIR}${PREFIX}/bin/ff2png

uninstall:
	@echo removing executable files from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/png2ff
	@rm -f ${DESTDIR}${PREFIX}/bin/ff2png

.PHONY: all options clean install uninstall
