#ifndef ircodegenh
#define ircodegenh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../symbol_table/symbol_table.h"
#include "../ast/ast.h"
#include "../type_system/type_system.h"

char* create_formatted_string(const char* format, ...); 
typedef struct { 
    char* buffer;
    size_t size;
    size_t capacity;
} CB;

CB* cb_init(int size);
CB* cb_resize(CB* cb, int needed);
CB* cb_append(CB* cb, const char* format, ...); 
CB* cb_concat(CB* dest, CB* src);
void cb_destroy(CB* cb);
void cb_write_to_file(CB* cb, FILE* fd);

typedef struct {
    char* name;
    Table* locals;
    CB* buffer;
} Proc;

void proc_destroy(Proc* proc);
Proc* proc_init(char* name, Table* st);

typedef struct {
    FILE* output;

    CB* perm;
    CB* text;

    Proc* curr_proc;
    Table* curr_scope;
    Table* global_table;

    int label_counter;
    int temp_counter;
} CG;

CG* cg_init(FILE* output, Table* gv);
void cg_destroy(CG* cg);
void cg_print(CG* cg);
Symbol* lookup_symbol(CG* cg, char* key);
#endif
