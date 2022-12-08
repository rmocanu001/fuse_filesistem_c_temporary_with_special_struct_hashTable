#define FUSE_USE_VERSION 25
#include <fuse.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "hash.h"
#include "search.h"

struct HASHTABLE * fs_table;

int lfs_getattr  ( const char *, struct stat * );
int lfs_readdir  ( const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info * );
int lfs_open     ( const char *, struct fuse_file_info * );
int lfs_read     ( const char *, char *, size_t, off_t, struct fuse_file_info * );
int lfs_release  ( const char *, struct fuse_file_info * );
int lfs_mkdir    ( const char *, mode_t);
int lfs_write    ( const char *, const char *, size_t, off_t, struct fuse_file_info *);
int lfs_mknod    ( const char *, mode_t, dev_t);
int lfs_truncate ( const char *, off_t);
int lfs_unlink (const char *);
int lfs_rename (const char * , const char *);
int lfs_release(const char *, struct fuse_file_info *); 
int lfs_utime (const char *, struct utimbuf *);

static struct fuse_operations lfs_oper = {
	.getattr	= lfs_getattr,
	.readdir	= lfs_readdir,
	.mknod      = lfs_mknod,
	.mkdir      = lfs_mkdir,
	.rmdir      = NULL,
    .unlink     = lfs_unlink,
	.truncate   = lfs_truncate,
	.open	    = lfs_open,
	.read	    = lfs_read,
	.release    = lfs_release,
	.write      = lfs_write,
	.utime      = lfs_utime,
    .destroy    = NULL, 
    .rename     = lfs_rename,
};

int lfs_utime (const char *path, struct utimbuf *time){
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    if (node == NULL) {
        char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments , path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);
    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * parent = find_node_from_path(fs_table, parent_path);
    initialize_file(parent,last_segment);
    struct lfs_file * file = (struct lsf_file*) node->entry;
    add_entry_to_table(fs_table, path, (void *) file);
    node = find_node_from_path(fs_table, path);
    }
    if (node->mode == 16877) {
        printf("nu stiu ce are boss");
        return -ENOENT;
    }
    struct lfs_file * file = (struct lsf_file*) node->entry;
    file->last_accessed=time->actime;
    file->last_modified=time->modtime;
    return 0;

}


int lfs_unlink (const char *path){
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
   
    if (node == NULL) { 
        printf("nu stiu ce are boss\n");
        return -ENOENT; 
    }

    if (node->mode == 16877) {
        printf("nu stiu ce are boss 2\n");
        return -ENOENT;
    }
    
    struct lfs_file * file = (struct lsf_file*) node->entry;

    //dezaloc file data si sterg intrarea din directoriul 
    
    struct lfs_directory* parent = file->parent_dir;
    
    for(int i=0; i<parent->num_files; i++)
    {
        if(strcmp(parent->files[i]->name,file->name)==0){
            printf("asta e numele: %s\n",file->name);
            for(int k=i;k<parent->num_files-1;k++){
                parent->files[k]=parent->files[k+1];
            }
            parent->num_files--;
            free(file->data);
            break;
        } 
    }
    
    remove_file_entry_to_table(fs_table,path,(void *) node->entry);
    return 0;

}

int lfs_rename (const char *path , const char *new_path){
     printf("rename: (path=%s)\n", path);

    if(strcmp(path, new_path)==0)
        return 0;
    // struct lfs_file * file_2 = initialize_file(parent_2, last_segment);
    // strcpy(file_2->data,file->data);
    // strcpy(file_2->name,last_segment_2);
    // add_entry_to_table(fs_table, new_path_dup, (void *) file_2);
    // node = find_node_from_path(fs_table, parent_path);
    // if (node->mode != 16877) {
    //     return -ENOENT;
    // }
    // struct lfs_directory * parent = (struct lfs_directory * ) node->entry;
    // remove_entry_to_table(fs_table,path_dup,(void *)file);

    char * new_path_dup = strdup(new_path);
    char * last_segment = strrchr(new_path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(new_path_dup) - strlen(last_segment));
    strncat(init_segments, new_path_dup, (strlen(new_path_dup) - strlen(last_segment)) - 1);
    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    struct lfs_file * file = (struct lsf_file*) node->entry;

    strcpy(file->name,last_segment);
    add_entry_to_table(fs_table, new_path, (void *) file);

    int res;

    return 0;
}

const char* get_file_path(const char * path){
    int i;
    int index=0;
    for(i = 0; i <= strlen(path); i++)
  	{
  		if(path[i] == '/')  
		{
  			index = i;	
 		}
	}
    const char*buff;
    if(index!=0)
    {
        strncpy(buff,path,index-1);
        return buff;
    }
    else return path;
    
}
const char* get_file_name(const char * path){
    int i;
    int index;
    for(i = 0; i <= strlen(path); i++)
  	{
  		if(path[i] == '/')  
		{
  			index = i;	
 		}
	}
    const char*buff=NULL;
    
    strcpy(buff,path+index);
    return buff;
    
}

lfs_file*find_file_name(lfs_directory*dir, char*name){
    //struct lfs_file*file=NULL;
    for(int i=0; i<dir->num_files; i++)
    {
        if(strcmp(dir->files[i]->name,name)==0){
            printf("asta e numele: %s\n",name);
            return dir->files[i];
        } 
   }
    return NULL;
}

int lfs_truncate(const char* path, off_t size){
     printf("truncate: (path=%s)\n", path);


    char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, parent_path);
    

    if (node == NULL) { return -ENOENT; } 
    
    if (node->mode != 16877) {
        return -ENOENT;
    }
   
    struct lfs_directory * parent = (struct lfs_directory * ) node->entry;

    struct lfs_file * file = find_file_name(parent,last_segment);

    file->data = realloc(file->data, size);
    file->size = size;
    file->last_modified = file->last_accessed;
    return size;
}

int lfs_write(const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi) {

    struct lfs_file * file = (struct lfs_file *) fi->fh;
    if (file == NULL) {
        fprintf(stderr, "write: Could not find any file at path '%s'\n", path);
        return -ENOENT;
    }

    int new_size;
    if (offset + size > file->size) {
        new_size = file->size + offset+size - file->size;
    } else {
        new_size = file->size + size - offset;
    }

    file->data = realloc(file->data, new_size);
    memcpy(file->data + offset, buffer, size);

    file->size = new_size;
    file->last_modified = file->last_accessed;
    return size;
}

int lfs_mkdir(const char * path, mode_t mode) {

    printf("mkdir: (path=%s)\n", path);

    char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, parent_path);
    

    if (node == NULL) { return -ENOENT; } 
    
    if (node->mode != 16877) {
        return -ENOENT;
    }
   
    struct lfs_directory * parent = (struct lfs_directory * ) node->entry;

    struct lfs_dir * dir = initialize_directory(parent, last_segment);
    add_entry_to_table(fs_table, path_dup, (void *) dir);
    return 0;
}

int lfs_mknod(const char * path, mode_t mode, dev_t rdev) {

    printf("mknod: (path=%s)\n", path);

    char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, parent_path);
    
    if (node == NULL) { return -ENOENT; } 
    
    if (node->mode != 16877) {
        return -ENOENT;
    }
   
    struct lfs_directory * parent = (struct lfs_directory * ) node->entry;

    struct lfs_file * file = initialize_file(parent, last_segment);
    add_entry_to_table(fs_table, path_dup, (void *) file);
    return 0;
}



int lfs_getattr( const char *path, struct stat *stbuf ) {
	int res = 0;

    printf("getattr: (path=%s)\n", path);

	memset(stbuf, 0, sizeof(struct stat));
    
	if( strcmp( path, "/" ) == 0) {
		stbuf->st_mode  = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
        return res;
	}

    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
    if (node == NULL) { return -ENOENT; } 

    if (node->mode == 16877) {
        stbuf->st_mode = 16877;
        struct lfs_directory * dir = (struct lfs_directory *) node->entry;
        stbuf->st_atime = dir->last_accessed;
        stbuf->st_mtime = dir->last_modified;
        stbuf->st_nlink = 2;

    } else if (node->mode == 33279){
        struct lfs_file * file = (struct lfs_file *) node->entry;
        stbuf->st_mode = file->mode;
        stbuf->st_size = file->size;
        stbuf->st_atime = file->last_accessed;
        stbuf->st_mtime = file->last_modified;
        stbuf->st_nlink = 1;
    } else {
        return -ENOENT;
    }

    return res;
}

int lfs_readdir( const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ) {
	(void) offset;
	(void) fi;
	printf("readdir: (path=%s)\n", path);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
    if (node == NULL) { return -ENOENT; } 
    
    if (node->mode != 16877) {
        return -ENOENT;
    }

    struct lfs_directory * dir = (struct lfs_directory *) node->entry;
    for (int i = 0; i < dir->num_directories; i++) {
        filler(buf, dir->directories[i]->name, NULL, 0);
    }

    for (int i = 0; i < dir->num_files; i++) {
        filler(buf, dir->files[i]->name, NULL, 0);
    }

	return 0;
}

//Permission
int lfs_open( const char *path, struct fuse_file_info *fi ) {

    printf("open: (path=%s)\n", path);

    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
    if (node == NULL) { return -ENOENT; } 

    if (node->mode == 16877) {
        return -ENOENT;
    }

    struct lfs_file * file = (struct lfs_file *) node->entry;
    file->last_accessed = time(NULL);
    fi->fh = (uint64_t) file;

	return 0;
}

int lfs_read( const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi ) {
    printf("read: (path=%s)\n", path);

    struct lfs_file * file = (struct lfs_file *) fi->fh;
    if (offset + size >= file->size + size) {
        return 0;
    }

    int bytes_to_read = file->size < size ? file->size : size;
    memcpy(buf, file->data + offset, bytes_to_read);
    printf("read %d bytes from file '%s'\n", bytes_to_read, path);
    return bytes_to_read;
}

int lfs_release(const char *path, struct fuse_file_info *fi) {
	printf("release: (path=%s)\n", path);

    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
    if (node == NULL) { return -ENOENT; } 

    if (node->mode == 16877) {
        return -ENOENT;
    }


	return 0;
}

int main( int argc, char *argv[] ) {

    fs_table = (struct HASHTABLE *) malloc(sizeof(struct HASHTABLE));
    fs_table->size = STD_FILE_SYSTEM_SIZE; 

    struct lfs_directory * root_directory = initialize_directory(NULL, "/");
    add_entry_to_table(fs_table, "/", (void *) root_directory);

	fuse_main( argc, argv, &lfs_oper);

	return 0;
}
