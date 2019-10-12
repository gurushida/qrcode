qrcode: main.c bitmatrix.c bitmatrix.h rgbimage.c rgbimage.h binarize.c binarize.h
	$(CC) -lpng main.c bitmatrix.c rgbimage.c binarize.c -o qrcode -Wall -Wextra -pedantic -std=c99

install: qrcode
	cp qrcode /usr/local/bin/

test: qrcode
	./qrcode images/225x225.png

