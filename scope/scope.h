#ifndef SCOPEH
#define SCOPEH
#include "../symbol_table/symbol_table.h"
#include <stdlib.h>

typedef struct Scope {
    Table* symbol_table;
    struct Scope* parent_scope;
}Scope;

Scope* create_scope(Table* symbol_table, Scope* parent_scope); 
void destroy_scope(Scope* parent_scope);

#endif
