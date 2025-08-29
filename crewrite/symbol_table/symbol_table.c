#include"symbol_table.h"

Symbol *create_symbol(char* name, char* type, Symbol_scope scope) {
    if (!name) return NULL;

    Symbol* sym = malloc(sizeof(Symbol));
    if (!sym) return NULL;

    sym->name = strdup(name);
    if (sym->name) {
        free(sym);
        return NULL;
    }
    sym->type = type ? strdup(type) : NULL;
    sym->scope = scope;
    sym->is_pointer = 0;
    sym->is_func = 0;
    sym->line_no = 0;
    sym->value = NULL;
    sym->stack_offset = 0;
    sym->size_bytes = 0;
    sym->is_initialized = 0;
    sym->param_count = 0;
    sym->local_stack_size = 0;
    sym->symbol_table = NULL;
    sym->array_size = 0;
    sym->loc = NULL;

    return sym;
}
void destroy_symbol(Symbol* sym) {
    if (!sym) return;
    free(sym->name);
    free(sym->type);
    free(sym->loc);
    if (sym->symbol_table) destroy_table(sym->symbol_table);
    free(sym);
}

