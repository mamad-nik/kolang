#ifndef SYMTABELH
#define SYMTABELH
#include "../type_system/type_system.h"
#include "../table/table.h"

typedef struct {
    char* name;
    char* loc;
    Type type;
    union {
        char byte_v;
        int int_v;
        int bool_v;
        double float_v;
        char* string_v;
    } value;
} Symbol;


#endif
