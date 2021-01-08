
# static dependencies : libbcrypt.a libbz2.a liblzma.a libz.a
CC = gcc
LIBS = -lzip -lbcrypt -lbz2 -llzma -lz


all:main
main:src/file_op.o
	$(CC) -shared $(wildcard src/*.c) $^ -o core.dll -s $(LIBS) -static
	# $(CC) -g $(wildcard src/*.c) $^ $(LIBS) -static
src/file_op.o:
	$(CC) -s $(wildcard src/io/*.c) -o $@ -c
