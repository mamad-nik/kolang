#ifndef TYPESYSH
#define TYPESYSH

#include<stdlib.h>
/*typedef enum { 
    TS_NO_TYPE,
    TS_INT_TYPE,
    TS_BOOL_TYPE,
    TS_STRING_TYPE,
    TS_FLOAT_TYPE,
    TS_BYTE_TYPE,
    TS_CUSTOM
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

