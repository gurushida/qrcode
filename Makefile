
SOURCES=bitmatrix.c rgbimage.c binarize.c finderpattern.c finderpatterngroup.c \
	qrcodefinder.c formatinformation.c versioninformation.c codewordmask.c codewords.c \
	blocks.c galoisfield.c reedsolomon.c polynomial.c bitstreamdecoder.c bitstream.c

qrcode: main.c $(SOURCES)
	$(CC) -lpng main.c $(SOURCES) -o qrcode -Wall -Wextra -pedantic -std=c99

qrcode_test: tests.c $(SOURCES)
	$(CC) -lpng tests.c $(SOURCES) -o qrcode_test -Wall -Wextra -pedantic -std=c99

install: qrcode
	cp qrcode /usr/local/bin/

test: qrcode_test
	./qrcode_test


