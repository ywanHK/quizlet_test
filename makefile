
# static dependencies : libbcrypt.a libbz2.a liblzma.a libz.a
# dynamic link option : -lzip
CC = gcc
LIBS = -lzip -lbcrypt -lbz2 -llzma -lz


all:main
main:src/rw.o
	$(CC) -shared $(wildcard src/*.c) $^ -o src/core.dll -g $(LIBS) -static
	# $(CC) -g $(wildcard src/*.c) $^ $(LIBS) -static
	rm -rf src/rw.o
src/rw.o:
	$(CC) -g $(wildcard src/io/*.c) -o $@ -c
	# $(CC) -g $(wildcard src/io/*.c) -o $@ -c
