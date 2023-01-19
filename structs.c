#include "struct.h"

static unsigned long inode_no=1;

// Djb2 hash function
unsigned long djb2_hash(char *str) {
        unsigned long hash = 5381;
        int c;
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        return hash;
}


int add_entry_to_table( HASHTABLE * table, char * path, void * entry) {

    unsigned long index = djb2_hash(path) % table->size;
    //creez instanta de has_node noua
     HASHTABLE_NODE * this = ( HASHTABLE_NODE * ) malloc(sizeof( HASHTABLE_NODE));

    this->mode = (( lfs_file * ) entry)->mode; //nu conteaza dc e lfs_directory
    this->entry = entry;
    this->path = path;
    this->is_linked = 0;
    this->next = NULL;
    //tratez coliziunile cu lista simplu inlantuita
    if (table->table[index] != NULL) {
         HASHTABLE_NODE * node = table->table[index];
        while (node->is_linked == 1) { node = node->next; }
        node->is_linked = 1;
        node->next = this;
    } else {
        table->table[index] = this;
    }
    
    return 0;
}
int remove_file_entry_to_table( HASHTABLE * table, char * path, void * entry) {

    unsigned long index = djb2_hash(path) % table->size;
     HASHTABLE_NODE * node = table->table[index];
    table->table[index]=table->table[index]->next;
    free(node);
    return 0;
}


 HASHTABLE_NODE * find_node_from_path( HASHTABLE * table, char * str) {

    unsigned long hash = djb2_hash(str);
    unsigned long clamped = hash % STD_FILE_SYSTEM_SIZE;

    /* No node with the given path exists in the filesystem */
    if (table->table[clamped] == NULL) {
        return NULL;
    }

     HASHTABLE_NODE * node = table->table[clamped];
    
    /* A node was found, this index has had no collisions
    and can simply be returned as is. */
    if (node->is_linked == 0) {
        return node;

    /* We found a node at an index where there has been collisions,
    iterate through the linked-list to find the node fitting our
    specific path and return it. */
    } else {
        while (strcmp(node->path, str) == 1) {
            if (node->is_linked == 1) {
                node = node->next;
            } else {
                return NULL;
            }
        }
        return node;
    }   

    return NULL;
} 


 lfs_file * initialize_file( lfs_directory * parent, char * name) {


     lfs_file * file = malloc(sizeof( lfs_file));

    file->parent_dir = parent;
    strcpy(file->name,name);
    file->data = (char *) malloc(0);
    file->size = 0;
    file->mode = S_IFREG | 0644;
    file->created_at    = time(NULL);
    file->last_accessed = file->created_at; 
    file->last_modified = file->created_at;
    file->links=1;
    file->inode=inode_no;
    parent->files[parent->num_files] = file; 
    parent->num_files++;

    inode_no++;
    return file;

}


 lfs_directory * initialize_directory( lfs_directory * parent, char * name) {
     lfs_directory * dir;
    dir = malloc(sizeof( lfs_directory));

    dir->parent_dir = parent;

    if (parent != NULL) {
        parent->directories[parent->num_directories] = dir;
        parent->num_directories++; 
    }

    

    dir->created_at    = time(NULL);
    dir->last_accessed = dir->created_at; 
    dir->last_modified = dir->created_at;

    dir->mode = S_IFDIR | 0744;
    strcpy(dir->name,name);
    dir->num_files = 0;
    dir->num_directories = 0;
    dir->links=2;
    dir->inode=inode_no;
    inode_no++;
    return dir; 
}
