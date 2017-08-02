/*
 * ISC-License
 *
 * (c) 2017 Laslo Hunhold <dev@frign.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef ARG_H
#define ARG_H

extern char *argv0;

/* int main(int argc, char *argv[]) */
#define ARGBEGIN for (argv0 = *argv, *argv ? (argc--, argv++) : ((void *)0);      \
                      *argv && (*argv)[0] == '-' && (*argv)[1]; argc--, argv++) { \
                 	int argparsed;                                            \
                 	if ((*argv)[1] == '-' && (*argv)[2] == '\0') {            \
                 		argc--, argv++;                                   \
                 		break;                                            \
                 	}                                                         \
                 	for (argparsed = 0, (*argv)++; (*argv)[0]; (*argv)++) {   \
                 		switch((*argv)[0])
#define ARGEND   		if (argparsed) {                                  \
                 			if ((*argv)[1] != '\0') {                 \
                 				break;                            \
                 			} else {                                  \
                 				argc--, argv++;                   \
                 				break;                            \
                 			}                                         \
                 		}                                                 \
                 	}                                                         \
                 }
#define ARGC()   *argv[0]
#define ARGF_(x) (((*argv)[1] == '\0' && !*(argv + 1)) ?       \
                 	(x) :                                  \
                 	(argparsed = 1, ((*argv)[1] != '\0') ? \
                 		(&(*argv)[1]) :                \
                 		(*(argv + 1))                  \
                 	)                                      \
                 )
#define EARGF(x) ARGF_(((x), exit(1), (char *)0))
#define ARGF()   ARGF_((char *)0)

#endif
