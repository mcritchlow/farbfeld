# farbfeld - tools to convert between png and ff
# See LICENSE file for copyright and license details

include config.mk

SRC = png2ff.c ff2png.c

all: png2ff ff2png

.c:
	@echo CC $<
	@${CC} -o $@ ${CFLAGS} ${LIBS} ${LDFLAGS} $<

clean:
	rm -f png2ff ff2png

install:
	@echo installing into ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f png2ff ff2png 2ff ${DESTDIR}${PREFIX}/bin

uninstall:
	@echo removing from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/png2ff
	@rm -f ${DESTDIR}${PREFIX}/bin/ff2png

.PHONY: all clean install uninstall
