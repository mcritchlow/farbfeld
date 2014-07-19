# imagefile - tools to convert to imagefile and back
# See LICENSE file for copyright and license details

include config.mk

SRC = png2imagefile.c imagefile2png.c
OBJ = ${SRC:.c=.o}

all: options png2imagefile imagefile2png

options:
	@echo imagefile build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c -o $@ ${CFLAGS} $<

${OBJ}: config.mk

png2imagefile: png2imagefile.o
	@echo CC -o $@
	@${CC} -o $@ png2imagefile.o ${LDFLAGS}

imagefile2png: imagefile2png.o
	@echo CC -o $@
	@${CC} -o $@ imagefile2png.o ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f png2imagefile imagefile2png ${OBJ}

install: all
	@echo installing executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f png2imagefile imagefile2png ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/png2imagefile
	@chmod 755 ${DESTDIR}${PREFIX}/bin/imagefile2png

uninstall:
	@echo removing executable files from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/png2imagefile
	@rm -f ${DESTDIR}${PREFIX}/bin/imagefile2png

.PHONY: all options clean install uninstall
