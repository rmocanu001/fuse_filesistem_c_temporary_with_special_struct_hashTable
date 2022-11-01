CC := cc
CFLAGS := -Wall
LDFLAGS := `pkg-config fuse --cflags --libs`

fsys: fuse-main.c
	$(CC) $(CFLAGS) fuse-main.c -o fsys $(LDFLAGS)
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
