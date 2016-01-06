# farbfeld - suckless image format with conversion tools
# See LICENSE file for copyright and license details
include config.mk

BIN = png2ff ff2png jpg2ff
SRC = ${BIN:=.c}
MAN1 = 2ff.1 ${BIN:=.1}
MAN5 = farbfeld.5

all: png2ff ff2png jpg2ff

.c:
	@echo CC $<
	@${CC} -o $@ ${CFLAGS} ${LIBS} ${LDFLAGS} $<

clean:
	@echo cleaning
	@rm -f ${BIN}

dist: clean
	@echo creating dist tarball
	@mkdir -p farbfeld-${VERSION}
	@cp -R FORMAT LICENSE Makefile README TODO config.mk \
		2ff ${SRC} ${MAN1} ${MAN5} farbfeld-${VERSION}
	@tar -cf farbfeld-${VERSION}.tar farbfeld-${VERSION}
	@gzip farbfeld-${VERSION}.tar
	@rm -rf farbfeld-${VERSION}

install: all
	@echo installing into ${DESTDIR}${PREFIX}/bin
	@mkdir -p "${DESTDIR}${PREFIX}/bin"
	@cp -f 2ff ${BIN} "${DESTDIR}${PREFIX}/bin"
	@echo installing manpages into ${DESTDIR}${MANPREFIX}
	@mkdir -p "${DESTDIR}${MANPREFIX}/man1"
	@cp -f ${MAN1} "${DESTDIR}${MANPREFIX}/man1"
	@cd "${DESTDIR}${MANPREFIX}/man1" && chmod 644 ${MAN1}
	@mkdir -p "${DESTDIR}${MANPREFIX}/man5"
	@cp -f ${MAN5} "${DESTDIR}${MANPREFIX}/man5"
	@cd "${DESTDIR}${MANPREFIX}/man5" && chmod 644 ${MAN5}

uninstall:
	@echo removing from ${DESTDIR}${PREFIX}/bin
	@cd "${DESTDIR}${PREFIX}/bin" && rm -f 2ff ${BIN}
	@echo removing manpages from ${DESTDIR}${MANPREFIX}
	@cd "${DESTDIR}${MANPREFIX}/man1" && rm -f ${MAN1}
	@cd "${DESTDIR}${MANPREFIX}/man5" && rm -f ${MAN5}

.PHONY: all clean dist install uninstall
