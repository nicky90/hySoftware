
CFLAG = -c -Wall -pg -g -DGF_INCLUDE_DATABASE -I../common -I.
DB = -lodbc
THREAD = -lpthread

.PHONY: all
all: gftm

gftm: gftm.o gftmconfig.o gftmfile.o gftmdb.o gftmgetlist.o gftmworkqueue.o ../common/gferror.o ../common/gfdbutils.o ../common/gfcompress.o ../common/gfchecksum.o ../common/md5.o ../common/gfsock.o
	gcc $^ -pg -o $@ $(DB) $(THREAD) -lz

gfinitdb: gfinitdb.o gftmdb.o ../common/gferror.o ../common/gfdbutils.o 
	gcc $^ -o $@ $(DB) $(THREAD) -lz


config: gftmconfig.o gftmdb.o ../common/gferror.o ../common/gfdbutils.o
	gcc $^ -o $@ $(DB)

%.o:%.c
	gcc $(CFLAG) $^ -o $@

../common/%.o:../common/%.c
	gcc $(CFLAG) $^ -o $@

.PHONY: clean
clean:
	rm *.o ../common/*.o gftm gfinitdb
