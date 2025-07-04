#include "type_system.h"

char* built_in_types[] = { "int", "bool", "string", "float", "byte" };
int size_bit = sizeof(built_in_types)/sizeof(built_in_types[0]);

Types* init_types() {
    Types* types = malloc(sizeof(Types));
    if (types == NULL) return NULL;
    types->elems = calloc(10, sizeof(Type*));
    if (types->elems == NULL) return NULL;
    types->no_elems = 0;
    types->cap = 10;
    for(int i = 0; i < size_bit; i++) {
        types = add_type(types, built_in_types[i]);
        if (types == NULL) return NULL;
    }
    return types;
}
Types* update_types(Types* types) {
    if (types == NULL) return NULL;
    if (types->no_elems != types->cap) return types;
    int new_size = types->cap+10;
    Type** tmp = reallocarray(types->elems, new_size, sizeof(Type*));
    if (tmp == NULL) return NULL;
    types->elems = tmp;
    types->cap = new_size;
    return types;
}
Types* add_type(Types* types, char* name) {
    if (name == NULL) return NULL;
    if (types == NULL) return NULL;

    Type* type =  malloc(sizeof(Type));
    if (type == NULL) return NULL;
    //TODO: add whatever you want
    type->name = name;

    types->elems[types->no_elems] = type;
    types->no_elems++;

    if (types->no_elems == types->cap) types = update_types(types);
    return types;
}
void destroy_types(Types* types) {
    if (types == NULL) return;
    for(int i=0; i < types->no_elems; i++) free(types->elems[i]);
    free(types->elems);
    free(types);
}


