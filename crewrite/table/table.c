#include "table.h"
#include <stdlib.h>
#include <string.h>

uint64_t hashf(char* data) {
    if(data == NULL) return (uint64_t)0;
    uint64_t hash = FNV_offset_basis; 
    while(*data) {
        hash *= FNV_prime;
        hash ^= *data;
        data++;
    }
    return hash;
}

Entry* create_entry(char *key, void* value) { 
    Entry* entry = (Entry*)malloc(sizeof(Entry));
    if (entry == NULL) return NULL;
    entry->key = strdup(key);
    entry->value = value;
    entry->next = NULL;
    return entry;
}
int add_entry(Table* table, Entry* entry) {
    if(table == NULL || entry == NULL) return 0;
    uint64_t hash = hashf(entry->key);
    int e = hash % table->no_buckets;
    if (table->entries[e] == NULL) table->entries[e] = entry;
    else {
        Entry* tmp = table->entries[e];
        table->entries[e] = entry;
        table->entries[e]->next = tmp;
    }
    return 1;
}

Table* update_table(Table* table) {
    int old_size = table->no_buckets;
    int new_size = old_size*2;
    Entry **new_entries = calloc(new_size, sizeof(Entry*));
    if(new_entries == NULL) return NULL;
    Entry **old_entries = table->entries;
    table->entries = new_entries;
    table->no_buckets = new_size;
    for(int i = 0; i < old_size; i++) {
        Entry* entry = old_entries[i];
        while(entry != NULL) {
            Entry* tmp = entry->next;
            entry->next = NULL;
            if(!add_entry(table, entry)) {
                table->entries = old_entries;
                table->no_buckets = old_size;
                free(new_entries);
                return NULL;
            }
            entry = tmp;
        }
    }
    free(old_entries);
    return table;
}

int insert_entry(Table* table, char *key, void* value) {
    if (key == NULL || value == NULL || table == NULL) return 0;
    Entry* entry = create_entry(key, value);
    if (entry == NULL) return 0; 
    if (!add_entry(table, entry)) return 0;
    table->no_entries++;
    if(table->no_entries/table->no_buckets >= 0.75) update_table(table);
    return 1;
}

Table* create_table() {
    Table* table = malloc(sizeof(Table));
    if (table == NULL) return NULL;
    table->entries = calloc(INIT_size, sizeof(Entry*));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }
    table->no_buckets = INIT_size;
    table->no_entries = 0;
    return table;
}

Entry* delete_entry_helper(Entry* entry) {
    if(entry == NULL) return NULL;
    Entry* next = entry->next;
    free(entry->key);
    free(entry);
    return next;
}
int delete_entry(Table* table, char* key) {
    if (key == NULL || table == NULL) return 0;
    uint64_t hash = hashf(key);
    int e = hash % table->no_buckets;
    if (table->entries[e] == NULL) return 0;
    else {
        if (strcmp(table->entries[e]->key, key) == 0) {
            table->entries[e] = delete_entry_helper(table->entries[e]);
            table->no_entries--;
            return 1;
        } 
        Entry* tmp = table->entries[e];
        while(tmp->next != NULL) {
            Entry* next = tmp->next;
            if (strcmp(next->key, key) == 0) {
                tmp->next = delete_entry_helper(next); 
                table->no_entries--;
                return 1;
            }
            tmp = next;
        }
    }
    return 0;
}
void destroy_table(Table* table) {
    for(int i = 0; i < table->no_buckets; i++) {
        Entry* entry = table->entries[i];
        do {
            entry = delete_entry_helper(entry);
        } while(entry != NULL);
    }
    free(table->entries);
    free(table);
}

Entry* lookup_entry(Table* table, char *key) {
    if (key == NULL || table == NULL) return NULL;
    uint64_t hash = hashf(key);
    int e = hash % table->no_buckets;
    if (table->entries[e] == NULL) return NULL;
    Entry* tmp = table->entries[e];
    while(tmp != NULL) {
        if(strcmp(tmp->key, key) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}
