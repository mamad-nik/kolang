#ifndef TYPESYSH
#define TYPESYSH

#include<stdlib.h>
#include<string.h>
#include"../ast/ast.h"

typedef enum { 
    TC_BASIC,
    TC_POINTER,
    TC_ARRAY,
    TC_STRUCT,
    TC_FUNCTION,
    TC_UNKNOWN
} Type_category;

typedef enum {
    BASIC_INT,
    BASIC_BOOL,
    BASIC_BYTE,
    BASIC_FLOAT,
    BASIC_STRING,
    BASIC_VOID,
} Basic_type;

struct Type_info; 

typedef struct {
    char* name;
    struct Type_info* type;
    int offset;
} Struct_field;

typedef struct Type_info {
    char* name;
    Type_category category;
    int size_byte;
    int alignment;

    union {
        Basic_type basic;
        struct {
            struct Type_info* pointed_to;
        } pointer;
        struct { 
            struct Type_info* element_type;
            int no_elements;
        } array;
        struct {
            int no_fields;
            Struct_field** fields;
        } structure;
        struct {
            char* name;
            struct Type_info* ret_type;
            int no_params;
            struct Type_info** params;
        } function;
    } data;
} Type_info; 

typedef struct {
    Type_info* basic_types[6];
    Type_info** custom_types;
    int no_custom;
    int capacity;
} Type_registry;

extern Type_registry* global_types;

//------
Type_registry* init_type_registry();
void destroy_type_registry(Type_registry* registry);
Type_registry* add_to_custom(Type_registry* tr, Type_info* ti);
//------
Type_info* get_basic_type(Basic_type basic);
int get_basic_type_size(Basic_type basic);
char* get_basic_type_name(Basic_type basic);
Basic_type parse_basic_type(char* name);
//------
Type_info* create_pointer_type(char* name, Type_info* pointed_to);
Type_info* create_array_type(char* name, Type_info* element_type, int no_elems);
Type_info* create_struct_type(char* name, Struct_field** fields, int no_fields);
Type_info* create_func_type(char* name, Type_info* ret_type, Type_info** params, int no_params);
Type_info* get_type_str(Type_registry* tr, char* name);

int type_check(Type_info* t1, Type_info* t2);

#endif

