#include"semantics.h"

typedef struct {
    char* name;
    Symbol* gt_entry_symbol;
    Table* st;
} Cproc;

Type_info* sem_argument(AST_node* tree);
int sem_statements(AST_node* tree);

Table* global_symbol_table;
Cproc* current_proc;
Symbol_scope scope; 

Cproc* create_cproc(char* name) {
    if (!name) return NULL;

    Table* table = create_table();
    if (!table) return NULL;

    Cproc* p = malloc(sizeof(Cproc));
    if (!p) {
        free(table);
        return NULL;
    }

    *p = (Cproc) {
        .name = strdup(name),
        .gt_entry_symbol = NULL,
        .st = table,
    };
    return p;
}

int switch_ast_type(AST_type id) {
    if (id == AST_INT || id == AST_BOOL || id == AST_STRING
            || id == AST_FLOAT || id == AST_BYTE) return 1;
    return 0;
}
void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

Struct_field* sem_struct_field(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_STRUCT_FIELD) return NULL;
    
    char *str = tree->left_child->value;
    Type_info* arg = sem_argument(tree->right_child);
    if (!arg) return NULL;

    Struct_field* field = malloc(sizeof(Struct_field));
    if (!field) return NULL;
    
    *field = (Struct_field) {
        .name = strdup(str),
        .type = arg,
        .offset = arg->size_byte
    };

    return field;
}
Type_info* sem_struct_def(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_STRUCT) return NULL;

    tree = tree->left_child;

    Struct_field** sfa = NULL;
    int offset = 0;
    int no_fields = 0; 
    while(tree) {
        if (tree->type == AST_SEQ) {
            if (tree->left_child) {
                Struct_field* sf = sem_struct_field(tree->left_child);    
                if (sf) return NULL;
                sf->offset += offset;
                offset = sf->offset;
                no_fields++;
                sfa = reallocarray(sfa, no_fields, sizeof(Struct_field*));
                if (!sfa) return NULL;

                sfa[no_fields-1] = sf;
            } 
        } else break;
    }
    char* name = malloc(sizeof(char)*10);
    rand_str(name, 10);
    Type_info*  structu =  create_struct_type(name, sfa, no_fields);
    if (!structu) return NULL;
    return structu;
}

Type_info* sem_argument_type(AST_node* tree) {
    if (!tree) return NULL;

    if (switch_ast_type(tree->type)) 
        return get_basic_type(map_basic_type(tree->type));
        
    if (tree->type == AST_STRUCT) return sem_struct_def(tree);

    if (tree->type == AST_SYMBOL) {
        Type_info* ti = get_type_str(tree->value);
        if (ti) return NULL;
        return ti;
    }
    return NULL;
}

Type_info* sem_argument_pointer(AST_node* tree) {
    if (!tree) return NULL;

    Type_info* output = NULL;

    if (tree->type == AST_POINTER) {
        char* name = malloc(sizeof(char)*10);
        rand_str(name, 10);
        Type_info* pointed_to =  sem_argument_pointer(tree->left_child);
        output = create_pointer_type(name, pointed_to);
        return output;
    } else {
        output = sem_argument_type(tree); 
        if (!output) return NULL;
        return output;
    }
    return output;
}

Type_info* sem_argument(AST_node* tree) {
    if (!tree) return NULL;
    
    Type_info* pointer = sem_argument_pointer(tree);
    if (!pointer) return NULL;
    
    return pointer;
}
void sem_gvar_def(AST_node* tree) {
    if (!tree) return;

    if (tree->type != AST_VAR) return;
    char* str  = tree->left_child->value;
    Type_info* type = sem_argument(tree->right_child); 

    Entry* entry = lookup_entry(global_symbol_table, str);
    if (!entry) {
        Symbol *symbol = create_symbol(str, type, scope);
        insert_entry(global_symbol_table, str, symbol);
        return;
    }
    Symbol* symbol = (Symbol*)entry->value; 
    if (type_check(symbol->type, type)) {
        return;
        //TODO: error
    } else return;
}

void sem_top_def(AST_node* tree) {
    if (!tree) return;
    AST_node* node = tree;
    scope = SCOPE_GLOBAL;

    while(node != NULL) {
        if (tree->type == AST_SEQ) {
            sem_gvar_def(node->left_child);
            node = node->right_child;
        }
    }
}

Type_info* sem_proc_out(AST_node* tree) {

    Symbol* symbol = current_proc->gt_entry_symbol;
    
    Type_info* type = NULL;
    if (!tree) type = get_basic_type(BASIC_VOID);
    else {
        if (switch_ast_type(tree->type)) {
            type = get_basic_type(map_basic_type(tree->type));
        } else if (tree->type == AST_SYMBOL) {
            type = get_type_str(tree->value);
            if (!type) return NULL;
        }
    }
    return type;
}

int traverse_params(AST_node* tree) {
    if (!tree) return -1;

    int nparams = 0;

    while (tree) {
        if (tree->type == AST_SEQ) {
            if (tree->left_child) nparams++;
            tree = tree->right_child;
        }
    }
    return nparams;
}

Type_info** sem_proc_inp(AST_node* tree, int nparams) {
    if (!tree) return NULL;
            
    Type_info** params = calloc(nparams, sizeof(Type_info *));

    for (int i = 0; i < nparams; i++) {
        if (tree) {
            if (tree->type == AST_SEQ) {
                if (tree->left_child) {
                    Type_info* param = sem_argument(tree->left_child);
                    //TODO: error
                    if (!param) return NULL;
                    params[i] = param;
                    Symbol *symbol = create_symbol(param->name, param, scope);
                    if(!insert_entry(current_proc->st, param->name, symbol)) return NULL;
                }
                tree = tree->right_child;
            }
        }
    }
    return params;
}

Type_info* sem_proc_inp_out(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_PROC_INPUT_OUTPUT) return NULL;

    scope = SCOPE_PARAMETER;
    if (!tree->left_child) return NULL;

    int nparams = traverse_params(tree->left_child);
    Type_info** params = sem_proc_inp(tree->left_child, nparams);
    if (!params) return NULL;

    Type_info* output = sem_proc_out(tree->right_child);
    if (!output) { 
        for (int i = 0; i < nparams; i++) {
            free(params[i]);
        }
        free(params);
        return NULL;
    }

    Type_info* proc = create_func_type(current_proc->name, output, params, nparams);
    if (!proc) { 
        free(output);
        for (int i = 0; i < nparams; i++) {
            free(params[i]);
        }
        free(params);
        return NULL;
    }

    return proc;
}

int sem_statements(AST_node* tree) {}

void sem_proc(AST_node* tree) {
    if (!tree) return;

    if (tree->type != AST_PROC) return;
    char* str = tree->value;

    current_proc = create_cproc(str);
    if (current_proc) return;

    AST_node* statements = tree->right_child;
    AST_node* head = tree->left_child;

    Type_info* proc = sem_proc_inp_out(head);
    if (!proc) return;
    
    int lss = sem_statements(statements);

    scope = SCOPE_GLOBAL;
    Symbol* symbol = create_symbol(str, proc, scope); 
    if (symbol) return;
    current_proc->gt_entry_symbol = symbol;

    symbol->is_func = 1;
    symbol->param_count = proc->data.function.no_params;
    symbol->symbol_table = current_proc->st;
    symbol->local_stack_size = lss;

    if(!insert_entry(global_symbol_table, str, symbol)) return;
}

void sem_var_def(AST_node* tree) {
    if (!tree) return;

    if (tree->type != AST_VAR) return;

    char* str = tree->left_child->value;
    Type_info* type = sem_argument(tree->right_child);

    Entry* entry = lookup_entry(current_proc->st, str);
    if (!entry) {
        Symbol *symbol = create_symbol(str, type, scope);
        insert_entry(current_proc->st, str, symbol);
        return;
    }
    Symbol* symbol = (Symbol*)entry->value; 
    if (type_check(symbol->type, type)) {
        return;
        //TODO: error
    } else return;
}
void semantics(AST_node* tree, Table* symbol_table) {}
    
