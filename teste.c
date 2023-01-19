#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <utime.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>


static char * mountpoint;


int main( int argc, char *argv[] )
{
    //printf("mountpoint path :%s\n",argv[argc-1]);
    mountpoint = realpath(argv[argc-1], NULL);
    //printf("\n\nmountpoint path :%s\n",mountpoint);
    strcat(mountpoint,"/test");
    mkdir(mountpoint,0755);
    rmdir(mountpoint); 
    return 0;
}