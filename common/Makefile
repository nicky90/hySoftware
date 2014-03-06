CC = gcc
CFLAGS = -c  -I. -I../gfbj/

objects = gfchecksum.o gfcompress.o gferror.o \
			gfsignal.o gfsock.o only1.o md5.o gfdbutils.o

.PHONY: all clean

all: $(objects)

gferror.o: gferror.c
	$(CC) -o $@ $(CFLAGS) $<
md5.o: md5.c
	$(CC) -o $@ $(CFLAGS) $<
gfchecksum.o: gfchecksum.c
	$(CC) -o $@ $(CFLAGS) $<
gfcompress.o: gfcompress.c
	$(CC) -o $@ $(CFLAGS) $< -lz
gfsock.o: gfsock.c
	$(CC) -o $@ $(CFLAGS) $<
gfsignal.o: gfsignal.c
	$(CC) -o $@ $(CFLAGS) $<
only1.o: only1.c
	$(CC) -o $@ $(CFLAGS) $<
gfdbutils.o: gfdbutils.c
	$(CC) -o $@ -DGF_INCLUDE_DATABASE $(CFLAGS) $<

clean:
	rm *.o
