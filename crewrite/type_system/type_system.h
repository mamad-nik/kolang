#ifndef TYPESYSH
#define TYPESYSH

#include<stdlib.h>
/*typedef enum { 
    NO_TYPE,
    INT_TYPE,
    BOOL_TYPE,
    STRING_TYPE,
    FLOAT_TYPE,
    BYTE_TYPE
} Type;*/

typedef struct {
    char* name;
    char* underlying_type;
} Type;

typedef struct {
    Type** elems;
    int no_elems;
    int cap;
} Types;

Types* init_types();
Types* add_type(Types* types, char* name);
void destroy_types(Types* types);

#endif

