objects=gfcompress.o gfsock.o gfchecksum.o md5.o gfsignal.o gfconfig.o gferror.o only1.o 
CC = gcc
CFLAG = -Wall -g -D_FILE_OFFSET_BITS=64 
all: gfserver
gfserver:gfserver.c gfserver.h gf_req_handler.c $(objects)
	$(CC) $(CFLAG) -o gfserver $(objects) gfserver.c gf_req_handler.c -lz -lpthread 
#gf_req_handler.o:gf_req_handler.c gfserver.h
#	$(CC) $(CFLAG) -c -o gf_req_handler.o gfserver.h
gfcompress.o:../common/gfcompress.c ../common/gfcompress.h 
	$(CC) $(CFLAG) -c -o gfcompress.o ../common/gfcompress.c -lz
gfsock.o:../common/gfsock.c ../common/gfsock.h 
	$(CC) $(CFLAG) -c -o gfsock.o ../common/gfsock.c
gfchecksum.o:../common/gfchecksum.c ../common/gfchecksum.h md5.o 
	$(CC) $(CFLAG) -c -o gfchecksum.o ../common/gfchecksum.c
md5.o:../common/md5.c ../common/md5.h
	$(CC) $(CFLAG) -c -o md5.o ../common/md5.c
gfsignal.o:../common/gfsignal.c ../common/gfsignal.h
	$(CC) $(CFLAG) -c -o gfsignal.o ../common/gfsignal.c
gfconfig.o:gfconfig.c gfconfig.h 
	$(CC) $(CFLAG) -c -o gfconfig.o gfconfig.c
gferror.o:../common/gferror.c ../common/gferror.h
	$(CC) $(CFLAG) -c -o gferror.o ../common/gferror.c
only1.o: ../common/only1.c
	$(CC) $(CFLAG) -c -o only1.o ../common/only1.c
clean:
	rm $(objects)
