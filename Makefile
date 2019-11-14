
SOURCES=bitmatrix.c rgbimage.c binarize.c finderpattern.c finderpatterngroup.c \
	qrcodefinder.c formatinformation.c versioninformation.c codewordmask.c codewords.c \
	blocks.c galoisfield.c reedsolomon.c polynomial.c bitstreamdecoder.c bitstream.c \
	eci.c bytebuffer.c shiftjis.c gb18030.c big5.c euc_kr.c qrcode.c

qrcode: main.c libqrcode.so
	$(CC) -lpng -lqrcode -L. main.c -Wl,-rpath,. -o qrcode -Wall -Wextra -pedantic -std=c99

qrcode_test: tests.c libqrcode.so
	$(CC) -lpng -lqrcode -L. tests.c -Wl,-rpath,. -o qrcode_test -Wall -Wextra -pedantic -std=c99

libqrcode.so: $(SOURCES)
	$(CC) -fPIC -lpng $(SOURCES) -shared -o libqrcode.so -Wall -Wextra -pedantic -std=c99

install: qrcode
	cp qrcode /usr/local/bin/

test: qrcode_test
	./qrcode_test

clean:
	rm qrcode qrcode_test libqrcode.so

