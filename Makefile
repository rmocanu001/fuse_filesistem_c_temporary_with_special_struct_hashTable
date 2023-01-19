CC := cc
CFLAGS := -Wall
LDFLAGS := `pkg-config fuse --cflags --libs`

CDFLAGS := `pkg-config fuse --cflags`

fsys: fuse-main.o structs.o
	$(CC) $(CFLAGS) fuse-main.o structs.o -o fsys $(LDFLAGS)

fuse-main.o: fuse-main.c struct.h
	$(CC) $(CFLAGS) $(CDFLAGS) -c fuse-main.c
struct.o: structs.c struct.h
	$(CC) $(CFLAGS) $(CDFLAGS) -c structs.c 
mnt_make: fsys
	mkdir mnt
	./fsys mnt
mnt_rm:
	umount mnt
	rm -r mnt
rapid: fsys mnt
	umount mnt
	rm -r mnt
	mkdir mnt
	./fsys mnt

teste: teste.c
	$(CC) $(CFLAGS) teste.c -o teste
