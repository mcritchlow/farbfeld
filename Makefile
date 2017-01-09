# farbfeld - suckless image format with conversion tools
# See LICENSE file for copyright and license details
include config.mk

BIN = png2ff ff2png jpg2ff ff2jpg ff2pam ff2ppm
SCRIPTS = 2ff
SRC = ${BIN:=.c}
HDR = arg.h
MAN1 = 2ff.1 ${BIN:=.1}
MAN5 = farbfeld.5

all: ${BIN}

${BIN}: ${@:=.o}

OBJ = ${SRC:.c=.o}

${OBJ}: config.mk ${HDR}

.o:
	${CC} ${CFLAGS} ${$*-LDFLAGS} -o $@ $<

.c.o:
	${CC} ${CFLAGS} ${$*-CFLAGS} ${CPPFLAGS} -c $<

clean:
	rm -f ${BIN} ${OBJ}

dist:
	rm -rf "farbfeld-${VERSION}"
	mkdir -p "farbfeld-${VERSION}"
	cp -R FORMAT LICENSE Makefile README TODO config.mk ${SCRIPTS} ${HDR} ${SRC} ${MAN1} ${MAN5} "farbfeld-${VERSION}"
	tar -cf - "farbfeld-${VERSION}" | gzip -c > "farbfeld-${VERSION}.tar.gz"
	rm -rf "farbfeld-${VERSION}"

install: all
	mkdir -p "${DESTDIR}${PREFIX}/bin"
	cp -f ${SCRIPTS} ${BIN} "${DESTDIR}${PREFIX}/bin"
	for f in $(BIN) $(SCRIPTS); do chmod 755 "${DESTDIR}${PREFIX}/bin/$$f"; done
	mkdir -p "${DESTDIR}${MANPREFIX}/man1"
	cp -f ${MAN1} "${DESTDIR}${MANPREFIX}/man1"
	for m in $(MAN1); do chmod 644 "${DESTDIR}${MANPREFIX}/man1/$$m"; done
	mkdir -p "${DESTDIR}${MANPREFIX}/man5"
	cp -f ${MAN5} "${DESTDIR}${MANPREFIX}/man5"
	for m in $(MAN5); do chmod 644 "${DESTDIR}${MANPREFIX}/man5/$$m"; done

uninstall:
	for f in $(BIN) $(SCRIPTS); do rm -f "${DESTDIR}${PREFIX}/bin/$$f"; done
	for m in $(MAN1); do rm -f "${DESTDIR}${MANPREFIX}/man1/$$m"; done
	for m in $(MAN5); do rm -f "${DESTDIR}${MANPREFIX}/man5/$$m"; done

.PHONY: all clean dist install uninstall
