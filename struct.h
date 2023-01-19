
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#define STD_DIRECTORY_TABLE_SIZE 1024
#define STD_DIRECTORY_FILES_SIZE 4096
#define STD_FILE_SYSTEM_SIZE     2048


typedef struct lfs_file {

    mode_t mode;
    struct lfs_directory * parent_dir;
    char  name[50];
    char * data;

    unsigned int size;
    time_t created_at;
    time_t last_accessed;
    time_t last_modified;
    nlink_t links;
    unsigned long inode;
    
} lfs_file;

typedef struct lfs_directory {

    mode_t mode;
    struct lfs_directory * parent_dir;
    char  name[50];
    struct lfs_file * files[STD_DIRECTORY_FILES_SIZE];
    struct lfs_directory * directories[STD_DIRECTORY_TABLE_SIZE];

    int num_files;
    int num_directories;

    time_t created_at;
    time_t last_accessed;
    time_t last_modified;
    nlink_t links;
    unsigned long inode;


} lfs_directory;

typedef struct HASHTABLE_NODE {

    mode_t mode; 
    void * entry;
    char * path;

    bool is_linked;
    struct HASHTABLE_NODE * next;
    
} HASHTABLE_NODE;

typedef struct HASHTABLE {
    int size;
    struct HASHTABLE_NODE * table[STD_FILE_SYSTEM_SIZE];

} HASHTABLE;

int add_entry_to_table( HASHTABLE * table, char * path, void * entry);
int remove_file_entry_to_table( HASHTABLE * table, char * path, void * entry);
unsigned long djb2_hash(char *str); 
HASHTABLE_NODE * find_node_from_path( HASHTABLE * table, char * str);
lfs_file * initialize_file( lfs_directory * parent, char * name);
lfs_directory * initialize_directory( lfs_directory * parent, char * name); 

