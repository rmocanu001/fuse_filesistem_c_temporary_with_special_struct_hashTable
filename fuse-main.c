#define FUSE_USE_VERSION 25
#include <fuse.h>
#include "struct.h"

struct lfs_directory * root_directory;
struct HASHTABLE * fs_table;

int my_getattr  ( const char *, struct stat * );
int my_readdir  ( const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info * );
int my_open     ( const char *, struct fuse_file_info * );
int my_read     ( const char *, char *, size_t, off_t, struct fuse_file_info * );
int my_release  ( const char *, struct fuse_file_info * );
int my_mkdir    ( const char *, mode_t);
int my_write    ( const char *, const char *, size_t, off_t, struct fuse_file_info *);
int my_mknod    ( const char *, mode_t, dev_t);
int my_truncate ( const char *, off_t);
int my_unlink (const char *);
int my_rename (const char * , const char *);
int my_release(const char *, struct fuse_file_info *); 
int my_utime (const char *, struct utimbuf *);
int my_rmdir (const char * );
int my_chmod (const char *, mode_t);
int my_create (const char *, mode_t , struct fuse_file_info*);
int my_readlink(const char *, char *, size_t );
int my_link(const char *from, const char *to);


static struct fuse_operations lfs_oper = {
	.getattr	= my_getattr,
	.readdir	= my_readdir,
	.mknod      = my_mknod,
	.mkdir      = my_mkdir,
	.rmdir      = my_rmdir,
    .unlink     = my_unlink,
	.truncate   = my_truncate,
	.open	    = my_open,
	.read	    = my_read,
	.release    = my_release,
	.write      = my_write,
	.utime      = my_utime,
    .destroy    = NULL, 
    .rename     = my_rename,
    .chmod      = my_chmod,
    .create     = my_create,
    .readlink   = my_readlink,
    .link       = my_link
    
};

int my_readlink(const char *path, char *buf, size_t size) {
  //char fpath[MAX_PATH_LEN];
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);

    if(!S_ISLNK(node->mode)){
        return -ENOENT; 
    }

    struct lfs_file * file = (struct lsf_file*) node->entry;
   
    char*full_path=NULL;
    strcpy(full_path,file->parent_dir);
    strcat(full_path,file->name);

    strncpy(buf,full_path,size);

  return 0;
}

int my_link(const char *from, const char *to) {
  struct HASHTABLE_NODE * node = find_node_from_path(fs_table, from);
    
    if (node == NULL) { return -ENOENT; } 
    
    if (!S_ISREG(node->mode)) {
        return -ENOENT;
    }

    char * path_dup = strdup(to);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node2 = find_node_from_path(fs_table, parent_path);

    struct lfs_directory * parent = (struct lfs_directory * ) node2->entry;


    struct lfs_file * file_from = (struct lsf_file*) node->entry;

    struct lfs_file * file = initialize_file(parent, last_segment);

    file->size=file_from->size;
    strcpy(file->data,file_from->data);
    file->links++;
    file_from->links++;
    //printf("\n\ninode: %d,       %d\n\n",file->inode,file_from->inode);
    file->inode=file_from->inode;
    printf("\n\ninode: %d,       %d\n\n",file->inode,file_from->inode);
    add_entry_to_table(fs_table, path_dup, (void *) file);
 
  return 0;
}

int my_chmod (const char *path, mode_t mode){
        struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    // if (S_ISDIR(node->mode)) {
    //     return -ENOENT;
    // }
    // nu recomand deoarece node->mode folosesc pentru a recunoaste in functii tipul de entitate regular_file/directory
    if (node == NULL) { 
        printf("nu stiu ce are boss\n");
        return -ENOENT; 
    }

    if (S_ISDIR(node->mode)) {
        struct lfs_directory * dir = (struct lfs_directory*) node->entry;
        //node->mode=mode;
        dir->mode=mode;
        dir->last_modified=time(NULL);
    }

    if (S_ISREG(node->mode)) {
        struct lfs_file * file = (struct lsf_file*) node->entry;
        //node->mode=mode;
        file->mode=mode;
        file->last_modified=time(NULL);
    }


    return 0;
}

int my_create (const char *path, mode_t mode, struct fuse_file_info *fi){
    printf("create: (path=%s)\n", path);

    char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, parent_path);
    
    if (node == NULL) { return -ENOENT; } 
    
    if (S_ISREG(node->mode)) {
        return -ENOENT;
    }
   
    struct lfs_directory * parent = (struct lfs_directory * ) node->entry;

    struct lfs_file * file = initialize_file(parent, last_segment);
    add_entry_to_table(fs_table, path_dup, (void *) file);
    fi->fh = (uint64_t) file;
    return 0;
}

int my_rmdir (const char * path)
{
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
   
    if (node == NULL) { 
        printf("nu stiu ce are boss\n");
        return -ENOENT; 
    }

    if (S_ISREG(node->mode)) {
        printf("face unlink, ce are boss 2\n");
        return my_unlink(path);
        return -ENOENT;
    }

    struct lfs_directory * file = (struct lfs_directory*) node->entry;
    struct lfs_directory* parent = file->parent_dir;
    printf("asta e numele: %s\n",file->name);
    // for(int i=0; i<file->num_directories; i++){
    //     char * new_path;
    //     strcpy(new_path,file->files[i]->parent_dir);
    //     if(file->files[i]->parent_dir!='/')
    //     strcat(new_path,"/");
    //     strcat(new_path,file->files[i]->name);
    //     printf("pana in rm @@@@@@");
    //     my_rmdir(new_path); 
    for(int i=0; i<parent->num_directories; i++)
    {
        if(strcmp(parent->directories[i]->name,file->name)==0){

            for(int k=i;k<parent->num_directories-1;k++){
                parent->directories[k]=parent->directories[k+1];
            }
            parent->num_directories--;
        }

    //}
    }
    for(int i=0; i<file->num_files; i++){
        char * new_path;
        strcpy(new_path,file->files[i]->parent_dir);
        strcat(new_path,"/");
        strcat(new_path,file->files[i]->name);
        my_unlink(new_path);
    }
    printf("face rmdir, ce are boss 2\n");

    remove_file_entry_to_table(fs_table,path,(void *) file);
}

int my_utime (const char *path, struct utimbuf *time){
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
    if (S_ISDIR(node->mode)) {
        printf("nu stiu ce are boss");
        return -ENOENT;
    }
    struct lfs_file * file = (struct lsf_file*) node->entry;
    file->last_accessed=time->actime;
    file->last_modified=time->modtime;
    return 0;

}


int my_unlink (const char *path){
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
   
    if (node == NULL) { 
        printf("nu stiu ce are boss\n");
        return -ENOENT; 
    }

    if (S_ISDIR(node->mode)) {
        printf("nu stiu ce are boss 2\n");
        return -ENOENT;
    }
    struct lfs_file * file=NULL;
    
    file = (struct lsf_file*) node->entry;

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

int my_rename (const char *path , const char *new_path){
     printf("rename: (path=%s)\n", path);

    if(strcmp(path, new_path)==0)
        return 0;
    // struct lfs_file * file_2 = initialize_file(parent_2, last_segment);
    // strcpy(file_2->data,file->data);
    // strcpy(file_2->name,last_segment_2);
    // add_entry_to_table(fs_table, new_path_dup, (void *) file_2);
    // node = find_node_from_path(fs_table, parent_path);
    // if (S_ISREG(node->mode)) {
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

int my_truncate(const char* path, off_t size){
     printf("truncate: (path=%s)\n", path);


    char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, parent_path);
    

    if (node == NULL) { return -ENOENT; } 
    
    if (S_ISREG(node->mode)) {
        return -ENOENT;
    }
   
    struct lfs_directory * parent = (struct lfs_directory * ) node->entry;

    struct lfs_file * file = find_file_name(parent,last_segment);

    file->data = realloc(file->data, size);
    file->size = size;
    file->last_modified = file->last_accessed;
    return size;
}

int my_write(const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi) {

    struct lfs_file * file = (struct lfs_file *) fi->fh;
    if (file == NULL) {
        fprintf(stderr, "write: Could not find any file at path '%s'\n", path);
        return -ENOENT;
    }
    if(((file->mode&S_IWUSR)==0)&&((file->mode&S_IWGRP)==0)&&((file->mode&S_IWOTH)==0)){
        printf("\n\n mode: %s \n",file->mode&S_IWUSR);
        return 0;
    }
    printf("\n\n%d\n\n",file->mode);
    printf("\n\niwusr:%d\n\n",S_IWUSR);


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

int my_mkdir(const char * path, mode_t mode) {

    printf("mkdir: (path=%s)\n", path);

    char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, parent_path);
    

    if (node == NULL) { return -ENOENT; } 
    
    if (S_ISREG(node->mode)) {
        return -ENOENT;
    }
   
    struct lfs_directory * parent = (struct lfs_directory * ) node->entry;

    struct lfs_dir * dir = initialize_directory(parent, last_segment);
    add_entry_to_table(fs_table, path_dup, (void *) dir);
    return 0;
}

int my_mknod(const char * path, mode_t mode, dev_t rdev) {

    printf("mknod: (path=%s)\n", path);

    char * path_dup = strdup(path);
    char * last_segment = strrchr(path_dup, '/') + 1;
    char * init_segments = (char *) malloc (strlen(path_dup) - strlen(last_segment));
    strncat(init_segments, path_dup, (strlen(path_dup) - strlen(last_segment)) - 1);

    char * parent_path = strcmp(init_segments, "") == 0 ? "/" : init_segments;
    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, parent_path);
    
    if (node == NULL) { return -ENOENT; } 
    
    if (S_ISREG(node->mode)) {
        return -ENOENT;
    }
   
    struct lfs_directory * parent = (struct lfs_directory * ) node->entry;

    struct lfs_file * file = initialize_file(parent, last_segment);
    add_entry_to_table(fs_table, path_dup, (void *) file);
    return 0;
}



int my_getattr( const char *path, struct stat *stbuf ) {
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

    if (S_ISDIR(node->mode)) {
        stbuf->st_mode = 16877;
        struct lfs_directory * dir = (struct lfs_directory *) node->entry;
        stbuf->st_atime = dir->last_accessed;
        stbuf->st_mtime = dir->last_modified;
        stbuf->st_nlink = dir->links;
        stbuf->st_ino=dir->inode;
    } else if (S_ISREG(node->mode)){
        struct lfs_file * file = (struct lfs_file *) node->entry;
        stbuf->st_mode = file->mode;
        stbuf->st_size = file->size;
        stbuf->st_atime = file->last_accessed;
        stbuf->st_mtime = file->last_modified;
        stbuf->st_nlink = file->links;
        stbuf->st_ino=file->inode;
    } 
    else {
        return -ENOENT;
    }

    return res;
}

int my_readdir( const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ) {
	(void) offset;
	(void) fi;
	printf("readdir: (path=%s)\n", path);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
    if (node == NULL) { return -ENOENT; } 
    
    if (S_ISREG(node->mode)) {
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
int my_open( const char *path, struct fuse_file_info *fi ) {

    printf("open: (path=%s)\n", path);

    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
    if (node == NULL) { return -ENOENT; } 

    if (S_ISDIR(node->mode)) {
        return -ENOENT;
    }

    struct lfs_file * file = (struct lfs_file *) node->entry;
    file->last_accessed = time(NULL);
    fi->fh = (uint64_t) file;

	return 0;
}

int my_read( const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi ) {
    printf("read: (path=%s)\n", path);

    struct lfs_file * file = (struct lfs_file *) fi->fh;


    printf("\n\n%o\n\n",file->mode);
    printf("\n\nirusr:%o\n\n",S_IRUSR);
    printf("\n\n mode: %o \n",file->mode&S_IRUSR);
    
    if(((file->mode&S_IRUSR)==0)&&((file->mode&S_IRGRP)==0)&&((file->mode&S_IROTH)==0)){
        printf("\n\n mode: %s \n",file->mode&S_IRUSR);
        return 0;
    }

    if (offset + size >= file->size + size) {
        return 0;
    }

    int bytes_to_read = file->size < size ? file->size : size;
    memcpy(buf, file->data + offset, bytes_to_read);
    printf("read %d bytes from file '%s'\n", bytes_to_read, path);
    return bytes_to_read;
}
#define PATH_MAX 250
static char *mounpoint;
static void bb_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, mounpoint);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will
				    // break here
}
int my_release(const char *path, struct fuse_file_info *fi) {
	printf("release: (path=%s)\n", path);

    struct HASHTABLE_NODE * node = find_node_from_path(fs_table, path);
    
    if (node == NULL) { return -ENOENT; } 

    if (S_ISDIR(node->mode)) {
        return -ENOENT;
    }

	return 0;
}

int main( int argc, char *argv[] ) {
    umask(0);
    fs_table = (HASHTABLE *) malloc(sizeof(HASHTABLE));
    fs_table->size = STD_FILE_SYSTEM_SIZE; 
    mounpoint = realpath(argv[argc-1], NULL);
    printf("\n\nmountpoint path :%s\n",mounpoint);
    lfs_directory * root_directory = initialize_directory(NULL, "/");
    add_entry_to_table(fs_table, "/", (void *) root_directory);
    printf("\n\nmountpoint path :%s\n",mounpoint);
	fuse_main( argc, argv, &lfs_oper);

	return 0;
}
