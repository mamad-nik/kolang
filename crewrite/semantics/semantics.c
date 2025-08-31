#include"semantics.h"

typedef struct {
    char* name;
    Symbol* gt_entry_symbol;
    Table* st;
} Cproc;

Type_info* sem_func_call(AST_node* tree);
Type_info* sem_struct_access(AST_node* tree);
Type_info* sem_argument(AST_node* tree);
Type_info* sem_array_def(AST_node* tree);
Type_info* sem_exp(AST_node* tree);
Type_info* sem_deref(AST_node* tree);
Type_info* sem_array_access(AST_node* tree);
Type_info* parse_struct_value(AST_node* tree);
Type_info* parse_array_value(AST_node* tree);
Symbol* sem_var_def(AST_node* tree);
int sem_if(AST_node* tree); 
int sem_assignment(AST_node* tree);
int sem_statements(AST_node* tree);
int sem_for(AST_node* tree); 
int sem_ret(AST_node* tree);

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

int switch_op(AST_type op)  {
    if (op == AST_PLUS || op == AST_MINUS || op == AST_TIMES ||
            op == AST_DIVIDE || op == AST_MODULO || op == AST_AND ||
            op == AST_OR || op == AST_EQ || op == AST_NEQ || op == AST_LT 
            || op == AST_GT || op == AST_LE || op == AST_GE || op == AST_EX_MARK) return 1; 
    return 0;
}

Symbol* check_for_a_type(char* str) { 
    if (!str) return NULL;

    Entry* entry = lookup_entry(current_proc->st, str);
    if (entry) {
        Symbol* symbol = (Symbol *) entry->value;
        return symbol;
    }
    entry = lookup_entry(global_symbol_table, str);
    if (entry) {
        Symbol* symbol = (Symbol *) entry->value;
        return symbol;
    }

    return NULL;
}

   
int switch_ast_type(AST_type id) {
    if (id == AST_INT || id == AST_BOOL || id == AST_STRING
            || id == AST_FLOAT || id == AST_BYTE) return 1;
    return 0;
}
int switch_ast_value(AST_type id) {
    if (id == AST_INTEGER_VAL || id == AST_BOOL_VAL || id == AST_STR_VAL
            || id == AST_FLOAT_VAL || id == AST_BYTE_VAL) return 1;
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
int traverse_fields(AST_node* tree) {
    if (!tree) return 0;
    
    int nfields = 0;
    while(tree) {
        if (tree->type == AST_SEQ) {
            if (tree->left_child) {
                nfields++;
            }
            tree = tree->right_child;
        }
    }
    return nfields;
}

Type_info* sem_struct_def(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_STRUCT) return NULL;

    tree = tree->left_child;

    int nfields = traverse_fields(tree);

    Struct_field** fields = calloc(nfields, sizeof(Struct_field*));
    if (!fields) return NULL;

    int offset = 0;
    for (int i = 0; i < nfields; i++) {
        if(tree) {
            if (tree->type == AST_SEQ) {
                if (tree->left_child) {
                    Struct_field* sf = sem_struct_field(tree->left_child);    
                    if (!sf) {
                        free(fields);
                        return NULL;
                    }
                    sf->offset += offset;
                    offset = sf->offset;
                    fields[i] = sf;
                } 
            } else break;
        }
    }
    char* name = malloc(sizeof(char)*10);
    rand_str(name, 10);
    Type_info*  structu =  create_struct_type(name, fields, nfields);
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
    if (entry) {
        return;
    }
    Symbol *symbol = create_symbol(str, type, scope);
    insert_entry(global_symbol_table, str, symbol);
    return;
}

int sem_is_num(Type_info* type) {
    if (!type) return 0;
    if (type->category == TC_BASIC)
        if (type->data.basic == BASIC_INT || type->data.basic == BASIC_FLOAT)
            return 1;
    return 0;
}

int sem_is_bool(Type_info* type) {
    if (!type) return 0;
    if (type->category == TC_BASIC)
        if (type->data.basic == BASIC_BOOL) return 1;
    return 0;
}

Type_info* sem_exp1(AST_node* tree) {
    if (!tree) return NULL;

    if(!switch_op(tree->type)) {
        if (switch_ast_value(tree->type))  {
            return get_basic_type(map_basic_val(tree->type));
        }

        if (tree->type == AST_LIB) tree = tree->left_child;
        Type_info* type = NULL;
        switch (tree->type){
            case(AST_STRUCT):
                type = sem_struct_access(tree);
                break;
            case(AST_PROC):
                type = sem_func_call(tree);
                break;
            case(AST_VALUE):
                type = sem_deref(tree);
                break;
            case(AST_ARRAY_ACCESS):
                type = sem_array_access(tree);
                break;
            case(AST_STRUCT_VALUE):
                type = parse_struct_value(tree);
                break;
            case(AST_ARRAY_VALUE):
                type = parse_array_value(tree);
                break;
            case(AST_SYMBOL): {
                Symbol* symbol = check_for_a_type(tree->value);
                if (symbol) type = symbol->type;
                break;
            }
            default:
                break;
        }
        if (!type) return NULL;
        return type;
    }

    if (tree->type == AST_PLUS || tree->type == AST_MINUS) {
        if (!tree->right_child) {
            if (!tree->left_child) return NULL;
            Type_info* type = sem_exp1(tree->left_child);
            if (!type) return NULL;
            if (!sem_is_num(type)) return NULL;
            return type;
        }
    }

    if (tree->type == AST_EX_MARK) {
        if (!tree->right_child) {
            if (!tree->left_child) return NULL;
            Type_info* type = sem_exp1(tree->left_child);
            if (!type) return NULL;
            if (!sem_is_bool(type)) return NULL;
            return type;
        }
    }

    AST_type t = tree->type;
    if (t == AST_PLUS || t == AST_MINUS || t == AST_TIMES || t == AST_DIVIDE
            || t == AST_MODULO || t == AST_GT || t == AST_LT ||
            t == AST_GE || t == AST_LE) {
        Type_info* lhs = sem_exp1(tree->left_child);
        if (!lhs) return NULL;
        Type_info* rhs = sem_exp1(tree->right_child);
        if (!rhs) return NULL;

        if (!sem_is_num(lhs) || !sem_is_num(rhs)) return NULL;
        if (type_check(lhs, rhs) != 1) return NULL;
        return lhs;

    } else if (t == AST_AND || t == AST_OR ) {
        Type_info* lhs = sem_exp1(tree->left_child);
        if (!lhs) return NULL;
        Type_info* rhs = sem_exp1(tree->right_child);
        if (!rhs) return NULL;

        if (!sem_is_bool(lhs) || !sem_is_bool(rhs)) return NULL;
        return lhs;

    } else if (t == AST_NEQ || t == AST_EQ)  {
        Type_info* lhs = sem_exp1(tree->left_child);
        if (!lhs) return NULL;
        Type_info* rhs = sem_exp1(tree->right_child);
        if (!rhs) return NULL;
        
        if (type_check(lhs, rhs) != 1) return NULL;
        
        if(lhs->category == TC_BASIC) return lhs;
        if(rhs->category == TC_BASIC) return rhs;
    }
            
    return NULL;
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
                    if (!param) {
                        free(params);
                        return NULL;
                    }
                    params[i] = param;
                    Symbol *symbol = create_symbol(param->name, param, scope);
                    if(!insert_entry(current_proc->st, param->name, symbol)){
                        free(params);
                        return NULL;
                    }
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

int sem_statement(AST_node* tree) {
    if (!tree) return 0;

    if (sem_assignment(tree)) return 1;
    if (sem_array_def(tree)) return 1;
    Symbol* symbol = sem_var_def(tree); 
    if (symbol) return insert_entry(current_proc->st, symbol->name, symbol);

    if (sem_if(tree)) return 1;
    if (sem_for(tree)) return 1;
    if (sem_func_call(tree)) return 1;
    if (sem_ret(tree)) return 1;

    return 0;
}
int sem_statements(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type != AST_STATEMENTS) return 0;

    tree = tree->left_child;
    while (tree) {
        if (tree->type ==  AST_SEQ) { 
            if (!tree->left_child) return 0;
            if(!sem_statements(tree->left_child)) return 0;
            tree = tree->right_child;
        }
    }
    return 1;
}

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
    
    scope = SCOPE_GLOBAL;
    Symbol* symbol = create_symbol(str, proc, scope); 
    if (!symbol) return;
    current_proc->gt_entry_symbol = symbol;

    int lss = sem_statements(statements);


    symbol->is_func = 1;
    symbol->param_count = proc->data.function.no_params;
    symbol->symbol_table = current_proc->st;
    symbol->local_stack_size = lss;

    if(!insert_entry(global_symbol_table, str, symbol)) return;
}

Symbol* sem_var_def(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_VAR) return NULL;

    char* str = tree->left_child->value;
    Type_info* type = sem_argument(tree->right_child);

    Symbol* symbol = check_for_a_type(str);
    if (symbol) {
        return NULL;
    }

    symbol = create_symbol(str, type, scope);
    if (!symbol) return NULL;
    return symbol;
}

Type_info* sem_array_access(AST_node* tree) {
    if(!tree) return NULL;
    
    if (tree->type != AST_ARRAY_ACCESS) return NULL;

    Symbol* symbol = check_for_a_type(tree->value);

    if (symbol->type->category != TC_ARRAY) return NULL;
    
    return symbol->type->data.array.element_type;   
}
Type_info* sem_deref_helper(AST_node* tree) {
    if(!tree) return NULL;

    if (tree->type ==  AST_VALUE) {
        Type_info* pointer = sem_deref_helper(tree->left_child);
        if(pointer->category != TC_POINTER) return NULL;
        return pointer->data.pointer.pointed_to;
    } else {
        if (tree->type == AST_LIB) tree = tree->left_child;

        Type_info* type = NULL; 
        switch (tree->type) {
            case (AST_ARRAY_ACCESS): 
                type = sem_array_access(tree);
                break;
            case (AST_STRUCT):
                type = sem_struct_access(tree);
                break;
            case (AST_PROC):
                type = sem_func_call(tree);
                break;
            case(AST_SYMBOL):
                Symbol* symbol = check_for_a_type(tree->value);
                if (!symbol) return NULL;
                type = symbol->type;
                break;
            default:
                return NULL;
                break;
        }
        return type;
    }
    return NULL;
}
Type_info* sem_deref(AST_node* tree) {
    if(!tree) return NULL;

    if (tree->type != AST_VALUE) return NULL;
    return sem_deref_helper(tree);
}

Type_info* sem_struct_access_helper(AST_node* tree, Type_info* par) {
    if (!tree || !par) return NULL;

    if (tree->type == AST_SEQ) {
        if(tree->left_child) {
            if(tree->left_child->type == AST_STRUCT_FIELD) {

                AST_node* node = tree->left_child;
                char *str = node->value;
                Type_info* type = NULL; 

                switch (node->type) {
                    case AST_ARRAY_ACCESS:
                        type = sem_array_access(tree);
                        break;
                    case AST_VALUE:
                        type = sem_deref(tree);
                        break;
                    case AST_STRUCT_FIELD:
                        break;
                    default:
                        return NULL;
                }
                
                int found = 0;
                for(int i = 0; i < par->data.structure.no_fields; i++) {
                    if (strcmp(par->data.structure.fields[i]->name, str) == 0) {
                        if (type) {
                            if (type_check(par->data.structure.fields[i]->type, type) != 1) return NULL;
                            found++;
                            break;
                        } else { 
                            type = par->data.structure.fields[i]->type;
                            found++;
                            break;
                        }
                    }
                }
                if (!found) return NULL;
                
                if (tree->right_child) return sem_struct_access_helper(tree->right_child, type);
                else return type;
            }
        }
    }
    return NULL;
}
Type_info* sem_struct_access(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_STRUCT) return NULL;

    char* str = tree->value;
    
    Symbol* symbol = check_for_a_type(tree->value);
    if (symbol->type->category != TC_STRUCT) return NULL;

    tree = tree->left_child;
    if (!tree) return NULL;
    return sem_struct_access_helper(tree->left_child, symbol->type);

}

int sem_if(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type == AST_IF_ELSE) {
        sem_if(tree->left_child);
        sem_if(tree->right_child);
    } else if (tree->type == AST_IF) {
        sem_statements(tree->right_child);
    } else if (tree->type == AST_ELSE) {
        int lss = sem_if(tree->left_child);
        if (lss) return lss;
        lss = sem_statements(tree->left_child);
        if (lss) return lss;
    }
    return 0;

}

int sem_for(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type != AST_FOR) return 0;

    return sem_statements(tree->right_child);
}

int sem_func_call_params(AST_node* tree, Type_info** params, int nparams) {
    if (!tree || !params) return 0;

    int i = 0;
    while(tree) {
        if (i >= nparams) return -1;
        if (!tree || tree->type != AST_SEQ) return 0;

        if (tree->left_child) i++;
        tree = tree->right_child;
    }
    if (i < nparams) return -1;
    return 1;
}
Type_info* sem_func_call(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_PROC) return NULL;

    Symbol *symbol = check_for_a_type(tree->value);
    if (!symbol) return NULL;

    if (symbol->type->category != TC_FUNCTION) return NULL;
    if (sem_func_call_params(tree->left_child, symbol->type->data.function.params, symbol->type->data.function.no_params) != 1) return NULL;
    return symbol->type->data.function.ret_type;
}
int sem_ret(AST_node* tree) {
    if (!tree) return 0;

    Symbol* symbol = current_proc->gt_entry_symbol;
    //XXX symbol->type->data.function.ret_type;
    return 1;
    
}
Type_info* sem_array_def(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_ARRAY) return NULL;

    int range = 0;
    if (tree->left_child) {
        if (tree->left_child->type != AST_ARRAY_RANGE) return NULL;
        range = atoi(tree->left_child->value);
    }
    Symbol* var = sem_var_def(tree->right_child);
    if (!var) return NULL;
   
    Type_info* array = create_array_type(var->name, var->type, range);
    if (!array) return NULL;

    if (!insert_entry(current_proc->st, var->name, array)) return NULL;

    return array;
}
void semantics(AST_node* tree, Table* symbol_table) {}
    
