qrcode: main.c bitmatrix.c bitmatrix.h rgbimage.c rgbimage.h binarize.c binarize.h \
        finderpattern.c finderpattern.h finderpatterngroup.c finderpatterngroup.h \
		qrcodefinder.c qrcodefinder.h formatinformation.c formatinformation.h \
		versioninformation.c versioninformation.h codewordmask.c codewordmask.h \
		codewords.c codewords.h blocks.c blocks.h galoisfield.c galoisfield.h
	$(CC) -lpng main.c bitmatrix.c rgbimage.c binarize.c finderpattern.c finderpatterngroup.c \
	qrcodefinder.c formatinformation.c versioninformation.c codewordmask.c codewords.c \
	blocks.c galoisfield.c \
	-o qrcode -Wall -Wextra -pedantic -std=c99

install: qrcode
	cp qrcode /usr/local/bin/

test: qrcode
	./qrcode images/225x225.png

