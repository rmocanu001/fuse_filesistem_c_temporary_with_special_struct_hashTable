
#include <string.h>
#include <stdbool.h>

#define STD_FILE_SYSTEM_SIZE     2048

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

// Djb2 hash function
unsigned long djb2_hash(char *str) {
        unsigned long hash = 5381;
        int c;
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        return hash;
}


int add_entry_to_table(struct HASHTABLE * table, char * path, void * entry) {

    unsigned long index = djb2_hash(path) % table->size;
    //creez instanta de has_node noua
    struct HASHTABLE_NODE * this = (struct HASHTABLE_NODE * ) malloc(sizeof(struct HASHTABLE_NODE));

    this->mode = ((struct lfs_file * ) entry)->mode; //nu conteaza dc e lfs_directory
    this->entry = entry;
    this->path = path;
    this->is_linked = 0;
    this->next = NULL;
    //tratez coliziunile cu lista simplu inlantuita
    if (table->table[index] != NULL) {
        struct HASHTABLE_NODE * node = table->table[index];
        while (node->is_linked == 1) { node = node->next; }
        node->is_linked = 1;
        node->next = this;
    } else {
        table->table[index] = this;
    }
    
    return 0;
}
int remove_file_entry_to_table(struct HASHTABLE * table, char * path, void * entry) {

    unsigned long index = djb2_hash(path) % table->size;
    struct HASHTABLE_NODE * node = table->table[index];
    table->table[index]=table->table[index]->next;
    free(node);
    return 0;
}



