#ifndef TABLEH
#define TABLEH
#include <stdint.h>

#define FNV_offset_basis 0xcbf29ce484222325ULL
#define FNV_prime 0x100000001b3ULL
#define INIT_size 64

typedef struct Entry {
    char* key;
    void* value;
    struct Entry* next;
} Entry;

typedef struct {
    Entry** entries;
    int no_entries;
    int no_buckets;
} Table;

uint64_t hashf(char* data);
Entry* create_entry(char *key, void* value);
int add_entry(Table* table, Entry* entry);
Table* update_table(Table* table);
int insert_entry(Table* table, char *key, void* value);
Table* create_table();
Entry* delete_entry_helper(Entry* entry);
int delete_entry(Table* table, char* key);
void destroy_table(Table* table);
Entry* lookup_entry(Table* table, char *key);
#endif 
