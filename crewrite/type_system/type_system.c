#include "type_system.h"
#define BASE_TR_SIZE 10
Type_registry* global_types = NULL;


Type_info* create_basic_type(Basic_type cat, int size, int align) {
    Type_info* ti = malloc(sizeof(Type_info));
    if (!ti) return NULL;
    *ti = (Type_info){
        .category = TC_BASIC,
        .size_byte = size, 
        .alignment = align,
        .data.basic = cat,
    };
    return ti;
}

Type_registry* init_type_registry() { 
    Type_registry* registry = malloc(sizeof(Type_registry));
    if (!registry) return NULL;

    registry->custom_types = calloc(BASE_TR_SIZE, sizeof(Type_info*));
    if (!registry->custom_types) {
        free(registry);
        return NULL;
    }

    registry->no_custom = 0;
    registry->capacity = BASE_TR_SIZE;

    registry->basic_types[BASIC_INT] = create_basic_type(BASIC_INT, 8, 8);
    if(!registry->basic_types[BASIC_INT]) {
        destroy_type_registry(registry);
        return NULL;
    }
    registry->basic_types[BASIC_BOOL] = create_basic_type(BASIC_BOOL, 1, 1);
    if(!registry->basic_types[BASIC_BOOL]) {
        destroy_type_registry(registry);
        return NULL;
    }

    registry->basic_types[BASIC_FLOAT] = create_basic_type(BASIC_FLOAT, 8, 8);
    if(!registry->basic_types[BASIC_FLOAT]) {
        destroy_type_registry(registry);
        return NULL;
    }

    registry->basic_types[BASIC_BYTE] = create_basic_type(BASIC_BYTE, 1, 1);
    if(!registry->basic_types[BASIC_BYTE]) {
        destroy_type_registry(registry);
        return NULL;
    }
    
    registry->basic_types[BASIC_STRING] = create_basic_type(BASIC_STRING, 8, 8);
    if(!registry->basic_types[BASIC_STRING]) {
        destroy_type_registry(registry);
        return NULL;
    }
    
    registry->basic_types[BASIC_VOID] = create_basic_type(BASIC_VOID, 0, 1);
    if(!registry->basic_types[BASIC_VOID]) {
        destroy_type_registry(registry);
        return NULL;
    }
    global_types = registry;
    return registry;
}

void destroy_type_registry(Type_registry* tr) {
    for (int i = 0; i < 6; i++) free(tr->basic_types[i]);
    for (int i = 0; i < tr->no_custom; i++){
        //TODO
        free(tr->custom_types[i]);
    }

    free(tr->custom_types);
    free(tr);
}

Type_registry* update_type_registry(Type_registry* tr) {
    if (!tr) return NULL;

    int new_size = tr->capacity + BASE_TR_SIZE;
    Type_info** new_customs = reallocarray(tr->custom_types, new_size, sizeof(Type_info*));
    if (!new_customs) return NULL;

    tr->custom_types = new_customs;
    tr->capacity = new_size;
    return tr;
}

Type_registry* add_to_custom(Type_info* ti) {
    if (!global_types || !ti) return NULL;

    for (int i = 0; i < global_types->no_custom; i++) {
        if (global_types->custom_types[i]->category == ti->category) {
            if (strcmp(global_types->custom_types[i]->name, ti->name) == 0) {
                if (type_check(global_types->custom_types[i], ti) != 1) {
                    return NULL;
                } else {
                    return global_types;
                }
            }
        }
    }

    if (global_types->no_custom + 1 >= global_types->capacity) {
        Type_registry* tempr = update_type_registry(global_types);
        if (!tempr) return NULL;
        global_types = tempr;
    }

    global_types->custom_types[global_types->no_custom] = ti;
    global_types->no_custom++;   
    return global_types;
}

Type_info* get_basic_type(Basic_type basic) {
    if (!global_types) return NULL;
    if (basic < 0 || basic >= 6) return NULL;
    return global_types->basic_types[basic];
}

char* get_basic_type_name(Basic_type basic) {
    switch (basic) {
        case BASIC_INT: return "int";
        case BASIC_BOOL: return "bool";
        case BASIC_FLOAT: return "float";
        case BASIC_BYTE: return "byte";
        case BASIC_STRING: return "string";
        case BASIC_VOID: return "void";
    }
    return NULL;
}
Basic_type parse_basic_type(char* name) {
    if (!name) return -1;
    if (strcmp(name, "int") == 0) return BASIC_INT;
    if (strcmp(name, "bool") == 0) return BASIC_BOOL;
    if (strcmp(name, "float") == 0) return BASIC_FLOAT;
    if (strcmp(name, "byte") == 0) return BASIC_BYTE;
    if (strcmp(name, "string") == 0) return BASIC_STRING;
    if (strcmp(name, "void") == 0) return BASIC_VOID;
    return -1;
}
Basic_type map_basic_type(AST_type id) {
    if (id == AST_INT) return BASIC_INT;
    if (id == AST_BOOL) return BASIC_BOOL;
    if (id == AST_FLOAT) return BASIC_FLOAT;
    if (id == AST_BYTE) return BASIC_BYTE;
    if (id == AST_STRING) return BASIC_STRING;
    return -1;
}
Basic_type map_basic_val(AST_type id) {
    if (id == AST_INTEGER_VAL) return BASIC_INT;
    if (id == AST_BOOL_VAL) return BASIC_BOOL;
    if (id == AST_STR_VAL) return BASIC_STRING;
    if (id == AST_FLOAT_VAL) return BASIC_FLOAT;
    if (id == AST_BYTE_VAL) return BASIC_BYTE;
    return BASIC_VOID;
}
int get_basic_type_size(Basic_type b) {
    return global_types->basic_types[b]->size_byte;
}

Type_info* create_pointer_type(char* name, Type_info* pointed_to) {
    if (!name || !pointed_to) return NULL;

    Type_info* ptr = malloc(sizeof(Type_info));
    if(!ptr) return NULL;

    char* str = strdup(name);
    if (!str) {
        free(ptr);
        return NULL;
    }
      

    *ptr = (Type_info){
        .name = str,
        .category = TC_POINTER,
        .size_byte = 8,
        .alignment = 8,
        .data.pointer.pointed_to = pointed_to,
    };
    return ptr;
}


Type_info* create_array_type(char* name, Type_info* element_type, int no_elems) {
     if (!name || !element_type || no_elems < 0) return NULL;

    Type_info* array = malloc(sizeof(Type_info));
    if(!array) return NULL;

    char* str = strdup(name);
    if (!str) {
        free(array);
        return NULL;
    }
    int align = element_type->alignment;

    *array = (Type_info){
        .name = str,
        .category = TC_ARRAY,
        .size_byte = 0,
        .alignment = align,
        .data.array = { 
            .element_type = element_type,
            .no_elements = no_elems,
        }
    };
    return array;
}

Type_info* create_struct_type(char* name, Struct_field** fields, int no_fields) {
     if (!name || !fields || no_fields < 1) return NULL;
     

    Type_info* strc = malloc(sizeof(Type_info));
    if(!strc) return NULL;
    
    char* str = strdup(name);
    if (!str) {
        free(strc);
        return NULL;
    }
    

    int tsize = 0;
    int talign = 0;
    for (int i = 0; i < no_fields; i++) {
        tsize += fields[i]->type->size_byte;
        talign = talign > fields[i]->type->alignment ? talign : fields[i]->type->alignment; 
    }

    *strc = (Type_info){
        .name = str,
        .category = TC_STRUCT,
        .size_byte = tsize,
        .alignment = talign,
        .data.structure = { 
            .no_fields = no_fields,
            .fields = fields
        }
    };
    return strc;
}


Type_info* create_func_type(char* name,
        Type_info* ret_type, Type_info** params, int no_params) {
    if (!name || !ret_type || no_params < 0) return NULL;

    Type_info* proc = malloc(sizeof(Type_info));
    if(!proc) return NULL;

    char* str = strdup(name);
    if (!str) {
        free(proc);
        return NULL;
    }

    *proc = (Type_info) {
        .name = str,
        .category = TC_FUNCTION,
        .size_byte = ret_type->size_byte,
        .alignment = 0,
        .data.function = {
            .ret_type = ret_type,
            .no_params = no_params,
            .params = params,
        }
    };
    return proc;
}

int struct_type_check(Type_info* t1, Type_info* t2) {
    if (!t1 || !t2) return -1;
    if (t1->category != TC_STRUCT || t2->category != TC_STRUCT) return -1;

    int name_check = 1;
    if (strcmp(t1->name, "") != 0 && strcmp(t2->name, "") != 0) 
        if (strcmp(t1->name, t2->name) != 0) name_check = 0;

    if (t1->data.structure.no_fields != t2->data.structure.no_fields) return -1;

    for(int i = 0; i < t1->data.structure.no_fields; i++) 
        if (type_check(t1->data.structure.fields[i]->type, 
                    t2->data.structure.fields[i]->type) != 1) return 0;
    
    return 1;
}
        

int type_check(Type_info* t1, Type_info* t2) {
    if (!t1 || !t2) return -1;

    switch (t1->category) {
        case TC_BASIC:
            if (t2->category == TC_BASIC)  return t1->data.basic == t2->data.basic;
            break;
        case TC_POINTER:
            if (t2->category == TC_POINTER) return type_check(t1->data.pointer.pointed_to, t2->data.pointer.pointed_to);
            break;
        case TC_ARRAY:
            if (t2->category == TC_ARRAY)  
                return type_check(t1->data.array.element_type, t2->data.array.element_type);
            break;
        case TC_STRUCT:
            return struct_type_check(t1, t2);
            break;
        case TC_FUNCTION:
            if (t2->category == TC_FUNCTION) 
                return type_check(t1->data.function.ret_type, t2->data.function.ret_type);
            break;
        default:
            return 0;
    }
    return 0;
}

Type_info* get_type_str(char* name) {
    if (!name || !global_types) return NULL;
    
    for (int i = 0; i < global_types->no_custom; i++) 
        if (strcmp(global_types->custom_types[i]->name, name) == 0) 
            return global_types->custom_types[i];
    return NULL;
    
}
