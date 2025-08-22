#ifndef SYMTABELH
#define SYMTABELH
#include "../type_system/type_system.h"
#include "../table/table.h"

typedef struct {
    char* name;
    char* loc;
    char* type;
    int is_func;
    int line_no;
    void* value;
} Symbol;


#endif
