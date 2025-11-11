#include "scope.h"

Scope* create_scope(Table* symbol_table, Scope* parent_scope) {
    if (!symbol_table) return NULL;

    Scope* scope = malloc(sizeof(Scope));
    if (!scope) return NULL;

    *scope = (Scope){ 
        .symbol_table = symbol_table,
        .parent_scope = parent_scope,
    };
    
    return scope;
}
void destroy_scope(Scope* scope) {
    if (!scope) return;

    free(scope);
}
