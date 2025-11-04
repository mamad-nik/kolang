#ifndef SYMTABELH
#define SYMTABELH
#include "../type_system/type_system.h"
#include "../table/table.h"
#include <string.h>
#include <stdlib.h>


typedef enum {
    SCOPE_GLOBAL,
    SCOPE_LOCAL,
    SCOPE_PARAMETER,
    SCOPE_TEMPORARY
} Symbol_scope;

typedef struct {
    char* name;
    char* loc;
    Type_info* type;
    int is_pointer;
    int is_func;
    int line_no;
    void* value;

    Symbol_scope scope;
    int stack_offset;
    int size_bytes;
    int is_initialized;
    int temp_no;

    int param_count;
    int local_stack_size;
    Table* symbol_table;

    int array_size;
} Symbol;

Symbol *create_symbol(char* name, Type_info* type, Symbol_scope scope);

#endif
