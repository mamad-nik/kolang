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
Type_info* sem_struct_value(AST_node* tree);
Type_info* sem_array_value(AST_node* tree);
Type_info* sem_assignment(AST_node* tree);
Symbol* sem_var_def(AST_node* tree);
int sem_statements(AST_node* tree);
int sem_if(AST_node* tree); 
int sem_for(AST_node* tree); 
int sem_ret(AST_node* tree);

Cproc* current_proc;
Symbol_scope scope; 
Table* global_symbol_table;

int type_compat(Type_info* type, Basic_type basic) {
    if (!type) return 0;

    Type_info* basic_type = get_basic_type(basic);
    if (!basic_type) return 0;
    return type_check(type, basic_type) == 1 ? 1 : 0;
}

void sem_panic(const char* msg) {
    if (current_proc)
        fprintf(stderr, "SEMANITCAL ANALISYS ERROR: %s in procedure named: %s\n", msg, current_proc->name);
    else 
        fprintf(stderr, "SEMANITCAL ANALISYS ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}
void sem_panic_parser_error() {
    sem_panic("undetected parser error");
}
void sem_panic_allocation() {
    sem_panic("allocation: there is nothing we can do");
}
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

Symbol* check_for_a_symbol(char* str) { 
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
    Type_info* arg = sem_argument(tree);
    if (!arg) sem_panic("invalid struct field");

    Struct_field* field = malloc(sizeof(Struct_field));
    if (!field) sem_panic_allocation();
    
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
    if (!fields) sem_panic_allocation();

    int offset = 0;
    for (int i = 0; i < nfields; i++) {
        if(tree) {
            if (tree->type == AST_SEQ) {
                if (tree->left_child) {
                    Struct_field* sf = sem_struct_field(tree->left_child);    
                    if (!sf) {
                        free(fields);
                        sem_panic("invalid struct field");
                        return NULL;
                    }
                    sf->offset += offset;
                    offset = sf->offset;
                    fields[i] = sf;
                } 
                tree = tree->right_child;
            } else break;
        }
    }
    char* name = malloc(sizeof(char)*10);
    if (!name) sem_panic_allocation();
    rand_str(name, 10);
    Type_info*  structu = create_struct_type(name, fields, nfields);
    if (!structu) sem_panic_allocation();
    return structu;
}

Type_info* sem_argument_type(AST_node* tree) {
    if (!tree) return NULL;

    if (switch_ast_type(tree->type)) 
        return get_basic_type(map_basic_type(tree->type));
        
    if (tree->type == AST_STRUCT) return sem_struct_def(tree);

    if (tree->type == AST_SYMBOL) {
        Type_info* ti = get_type_str(tree->value);
        if (!ti) sem_panic("undefined type");
        return ti;
    }
    return NULL;
}

Type_info* sem_argument_pointer(AST_node* tree) {
    if (!tree) return NULL;

    Type_info* output = NULL;

    if (tree->type == AST_POINTER) {
        char* name = malloc(sizeof(char)*10);
        if (!name) sem_panic_allocation();
        rand_str(name, 10);
        if (!name) sem_panic_allocation();
        Type_info* pointed_to = sem_argument_pointer(tree->left_child);
        output = create_pointer_type(name, pointed_to);
        return output;
    } else {
        output = sem_argument_type(tree); 
        if (!output) sem_panic("invalid argument type");
        return output;
    }
    return output;
}

Type_info* sem_argument(AST_node* tree) {
    if (!tree) return NULL;
    
    if (!tree->right_child) sem_panic_parser_error();
    tree = tree->right_child; 
    Type_info* pointer = sem_argument_pointer(tree);
    if (!pointer) sem_panic("invalid argument type");
    
    return pointer;
}

Type_info* sem_type_def(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_TYPE) return NULL;
    Type_info* type = sem_argument(tree);
    if (!type) sem_panic_allocation();

    char* str = strdup(tree->left_child->value);
    if (!str) sem_panic_allocation();
    type->name = str;
    if(!add_to_custom(type)) sem_panic_allocation();
    return type;
}
void sem_gvar_def(AST_node* tree) {
    if (!tree) return;

    if (tree->type != AST_VAR) return;
    char* str  = tree->left_child->value;
    Type_info* type = sem_argument(tree); 

    Entry* entry = lookup_entry(global_symbol_table, str);
    if (entry) {
        sem_panic("redifenition of an existing variable");
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

Struct_field* create_struct_field(char* name, Type_info* type) {
    if (!name || !type ) return NULL;

    Struct_field* sf = malloc(sizeof(Struct_field));
    if (!sf) sem_panic_allocation();

    *sf = (Struct_field) {
        .name = strdup(name),
        .type = type,
        .offset = 0
    };

    return sf;
}
Type_info* sem_struct_lit_assign(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_ASSIGN) return NULL;

    if (!tree->left_child || !tree->right_child) return NULL;
    AST_node* lhs = tree->left_child;
    AST_node* rhs = tree->right_child;

    if (lhs->type != AST_SYMBOL) sem_panic("invalid left hand side expression in struct value");
    char* str = strdup(lhs->value);
    if (!str) sem_panic_allocation();


    Type_info* type = sem_exp(rhs);
    if (!type) sem_panic("invalid right hand side expression in struct value");

    type->name = str;
    return type;
    
}
Struct_field* sem_struct_lit(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_STRUCT_FIELD) return NULL;

    if (!tree->left_child) sem_panic_parser_error();
    tree = tree->left_child;

    if (tree->type == AST_ASSIGN) {
        Type_info* type = sem_struct_lit_assign(tree);
        if (!type) sem_panic_parser_error();
        return create_struct_field(type->name, type);
    } else {
        Type_info* type = sem_exp(tree);
        if (!type) return NULL;
        return create_struct_field("", type);
    }
    return NULL;
    
}
Type_info* sem_struct_value(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_STRUCT_VALUE) return NULL;
    char* str = NULL;
    if (strcmp(tree->value, "") > 0) {
        str = strdup(tree->value);
        Type_info* type = get_type_str(str);
        if (!type) return NULL;
    }


    if(!tree) return NULL;
    tree = tree->left_child;

    int nfields = traverse_fields(tree);

    Struct_field** fields = calloc(nfields, sizeof(Struct_field*));
    if (!fields) sem_panic_allocation();

    for (int i = 0; i < nfields; i++) {
        if (tree->type != AST_SEQ) sem_panic_parser_error(); 
        Struct_field* sf = sem_struct_lit(tree->left_child);
        fields[i] = sf;
        tree = tree->right_child;
    }

    if (!str) {
        str = malloc(sizeof(char)*10);
        if (!str) sem_panic_allocation();
        rand_str(str, 10);
    }
    Type_info* type = create_struct_type(str, fields, nfields);
    if (!type) sem_panic_allocation();
    return type;

}
Type_info* sem_array_value(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_ARRAY_VALUE) return NULL;
    if (!tree->left_child) sem_panic_parser_error();
    tree = tree->left_child;

    int no_elements = 0;
    Type_info* type = NULL;
    while(tree) {
        if (tree->type == AST_SEQ) {
            if(tree->left_child) {
                if (tree->left_child->type == AST_ARRAY_LIT) {
                    Type_info* elem_type = sem_exp(tree->left_child);
                    if (!elem_type) sem_panic("invalid expression in array value");
                    if (!type) {
                        type = elem_type;
                    } else {
                        if (type_check(type, elem_type) != 1) 
                            sem_panic("incompatible types in array value");
                        no_elements++;
                    }
                }
            }
            tree = tree->right_child;
        }
    }
    if (type == NULL) return NULL;
    char* name = malloc(sizeof(char)*10);
    if (!name) sem_panic_allocation();
    rand_str(name, 10);
    Type_info* array = create_array_type(name, type, no_elements);
    if (!array) return NULL;
    return array;
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
                type = sem_struct_value(tree);
                break;
            case(AST_ARRAY_VALUE):
                type = sem_array_value(tree);
                break;
            case(AST_SYMBOL): {
                Symbol* symbol = check_for_a_symbol(tree->value);
                if (symbol) type = symbol->type;
                break;
            }
            default:
                break;
        }
        if (!type) sem_panic("invalid expression");
        return type;
    }

    if (tree->type == AST_PLUS || tree->type == AST_MINUS) {
        if (!tree->right_child) {
            if (!tree->left_child) sem_panic_parser_error();
            Type_info* type = sem_exp1(tree->left_child);
            if (!type) sem_panic_parser_error();
            if (!sem_is_num(type)) sem_panic("invalid operand to a unary operator");
            return type;
        }
    }

    if (tree->type == AST_EX_MARK) {
        if (!tree->right_child) {
            if (!tree->left_child) sem_panic_parser_error();
            Type_info* type = sem_exp1(tree->left_child);
            if (!type) sem_panic_parser_error();
            if (!sem_is_bool(type)) sem_panic("invalid operand to a unary operator");
            return type;
        }
    }

    AST_type t = tree->type;
    if (t == AST_PLUS || t == AST_MINUS || t == AST_TIMES || t == AST_DIVIDE
            || t == AST_MODULO || t == AST_GT || t == AST_LT ||
            t == AST_GE || t == AST_LE) {
        Type_info* lhs = sem_exp1(tree->left_child);
        if (!lhs) sem_panic("invalid left hand side of a arithmatic operator");
        Type_info* rhs = sem_exp1(tree->right_child);
        if (!rhs) sem_panic("invalid right hand side of a arithmatic operator");

        if (!sem_is_num(lhs) || !sem_is_num(rhs)) 
            sem_panic("either side of a arithmatic operation is not a number");
        if (type_check(lhs, rhs) != 1) 
            sem_panic("can't add float to int. also implicit typing is not done.");
        return lhs;

    } else if (t == AST_AND || t == AST_OR ) {
        Type_info* lhs = sem_exp1(tree->left_child);
        if (!lhs) sem_panic("invalid left hand side of a boolean operator");
        Type_info* rhs = sem_exp1(tree->right_child);
        if (!rhs) sem_panic("invalid right hand side of a boolean operator");

        if (!sem_is_bool(lhs) || !sem_is_bool(rhs)) 
            sem_panic("either side of a boolean operation is not a boolean expression");
        return lhs;

    } else if (t == AST_NEQ || t == AST_EQ)  {
        Type_info* lhs = sem_exp1(tree->left_child);
        if (!lhs) sem_panic("invalid left hand side of a equality operator");
        Type_info* rhs = sem_exp1(tree->right_child);
        if (!rhs) sem_panic("invalid right hand side of a equality operator");
        
        if (type_check(lhs, rhs) != 1)
            sem_panic("both sides of a equality operation must have same types"); 
        
        if(lhs->category == TC_BASIC) return lhs;
        if(rhs->category == TC_BASIC) return rhs;
        return lhs;
    }
            
    return NULL;
}
Type_info* sem_exp(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type == AST_EXPR) {
        if (!tree->left_child) sem_panic_parser_error();
        else tree = tree->left_child;
    }
    return sem_exp1(tree);
}
void sem_top_def(AST_node* tree) {
    if (!tree) return;
    scope = SCOPE_GLOBAL;
    
    if (tree->type != AST_TOP_DEF) return;
    tree = tree->left_child;

    while(tree) {
        if (tree->type == AST_SEQ) {
            AST_node* node = tree->left_child;
            switch(node->type) {
                case(AST_VAR):
                    sem_gvar_def(node);
                    break;
                case(AST_TYPE):
                    sem_type_def(node);
                    break;
                default:
                    break;
            }
            tree = tree->right_child;
        } else sem_panic_parser_error();

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
            if (!type) sem_panic("undefined type in procedure output");
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
    if (!params) sem_panic_allocation();

    for (int i = 0; i < nparams; i++) {
        if (tree) {
            if (tree->type == AST_SEQ) {
                if (tree->left_child) {
                    Type_info* param = sem_argument(tree->left_child);
                    if (!param) {
                        free(params);
                        sem_panic("invalid argument in procedure definition");
                    }
                    params[i] = param;
                    Symbol *symbol = create_symbol(param->name, param, scope);
                    if(!insert_entry(current_proc->st, param->name, symbol)){
                        free(params);
                        sem_panic_allocation();
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
    int nparams = 0;
    scope = SCOPE_PARAMETER;
    Type_info** params = NULL;
    if (tree->left_child) {
        nparams = traverse_params(tree->left_child);
        params = sem_proc_inp(tree->left_child, nparams);
        if (!params) sem_panic("invalid procedure input");
    }

    Type_info* output = sem_proc_out(tree->right_child);
    if (!output) { 
        for (int i = 0; i < nparams; i++) {
            free(params[i]);
        }
        free(params);
        sem_panic("invalid procedure output");
    }

    Type_info* proc = create_func_type(current_proc->name, output, params, nparams);
    if (!proc) { 
        free(output);
        for (int i = 0; i < nparams; i++) {
            free(params[i]);
        }
        free(params);
        sem_panic_allocation();
    }

    return proc;
}

Type_info* sem_place(AST_node* tree) {
    if (!tree) return NULL;

    Type_info* type = NULL;
    switch (tree->type) {
        case (AST_ARRAY):
            type = sem_array_def(tree);
            break;
        case (AST_VAR): {
            Symbol* symbol = sem_var_def(tree);
            if(symbol) type = symbol->type;
            break;
        }
        case (AST_STRUCT):
            type = sem_struct_access(tree);
            break;
        case (AST_ARRAY_ACCESS):
            type = sem_array_access(tree);
            break;
        case (AST_VALUE):
            type = sem_deref(tree);
            break;
        case (AST_SYMBOL): {
            Symbol* symbol = check_for_a_symbol(tree->value);
            if(symbol) type = symbol->type;
            break;
        }
        default:
            break;
    }
    if (!type) return NULL;
    return type;
}

Type_info* sem_assignment(AST_node* tree) {
    if (!tree) return NULL;
    
    if (tree->type != AST_ASSIGN) return NULL;
    
    Type_info* lhs = sem_place(tree->left_child);
    if (!lhs) sem_panic("invalid left hand side to an assignment");

    Type_info* rhs = sem_exp(tree->right_child);
    if (!rhs) sem_panic("invalid right hand side to an assignment");
    if (type_check(rhs, lhs) != 1) sem_panic("incompatible types in assignment");
    return rhs;
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
            if(!sem_statement(tree->left_child)) 
                sem_panic("invalid statement");
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
    if (!current_proc) sem_panic_allocation();

    AST_node* statements = tree->right_child;
    AST_node* head = tree->left_child;

    Type_info* proc = sem_proc_inp_out(head);
    if (!proc) return;
    
    scope = SCOPE_GLOBAL;
    Symbol* symbol = create_symbol(str, proc, scope); 
    if (!symbol) sem_panic_allocation();
    current_proc->gt_entry_symbol = symbol;

    scope = SCOPE_LOCAL;
    if (!sem_statements(statements)) sem_panic("invalid statements");

    symbol->is_func = 1;
    symbol->param_count = proc->data.function.no_params;
    symbol->symbol_table = current_proc->st;
    symbol->local_stack_size = 0;

    if(!insert_entry(global_symbol_table, str, symbol)) sem_panic_allocation();
}
void sem_procs(AST_node* tree) {
    if (!tree) return;

    while(tree) {
        if (tree && tree->left_child) 
            if(tree->type == AST_SEQ)
                sem_proc(tree->left_child);
        tree = tree->right_child;
    }

}
Symbol* sem_var_def(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_VAR) return NULL;

    char* str = tree->left_child->value;
    Type_info* type = sem_argument(tree);
    if (!type) sem_panic("undefined type in variable definition");

    Symbol* symbol = check_for_a_symbol(str);
    if (symbol) sem_panic("redefinition of an existing variable");

    symbol = create_symbol(str, type, scope);
    if (!symbol) sem_panic_allocation();
    if (!insert_entry(current_proc->st, str, symbol)) sem_panic_parser_error();
    return symbol;
}

Type_info* sem_array_access(AST_node* tree) {
    if(!tree) return NULL;
    
    if (tree->type != AST_ARRAY_ACCESS) return NULL;

    Symbol* symbol = check_for_a_symbol(tree->value);

    if (symbol->type->category != TC_ARRAY) return NULL;
    
    return symbol->type->data.array.element_type;   
}
Type_info* sem_deref_helper(AST_node* tree) {
    if(!tree) return NULL;

    if (tree->type == AST_VALUE) {
        Type_info* pointer = sem_deref_helper(tree->left_child);
        if(pointer->category != TC_POINTER) sem_panic("dereferencing of a non pointer value");
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
                Symbol* symbol = check_for_a_symbol(tree->value);
                if (!symbol) sem_panic("undefined variable");
                type = symbol->type;
                break;
            default:
                sem_panic("invalid value to dereferening");
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

Type_info* sem_struct_access_deref_helper(AST_node* tree) {
    if(!tree) return NULL;

    if (tree->type == AST_VALUE) {
        Type_info* pointer = sem_deref_helper(tree->left_child);
        if(pointer->category != TC_POINTER) sem_panic("dereferencing of a non pointer value");
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
                Symbol* symbol = check_for_a_symbol(tree->value);
                if (!symbol) sem_panic("undefined variable");
                type = symbol->type;
                break;
            default:
                sem_panic("invalid value to dereferening");
                break;
        }
        return type;
    }
    return NULL;
}
Type_info* sem_struct_access_deref(AST_node* tree) {
    if(!tree) return NULL;

    if (tree->type != AST_VALUE) return NULL;
    return sem_deref_helper(tree);
}

Type_info* sem_struct_access_helper(AST_node* tree, Type_info* par) {
    if (!tree || !par) return NULL;

    Type_info* type = NULL; 
    AST_node* next = NULL; 

    if (tree->type == AST_SEQ) {
        next = tree->right_child;
        if(!tree->left_child) sem_panic_parser_error();
        tree = tree->left_child;
    }
    if(tree->type != AST_STRUCT_FIELD) return NULL;
    if(!tree->left_child) sem_panic_parser_error();
    tree = tree->left_child;

    char *str = tree->value;

    switch (tree->type) {
        case AST_ARRAY_ACCESS:
            type = sem_array_access(tree);
            break;
        case AST_VALUE:
            type = sem_deref(tree);
            break;
        case AST_SYMBOL:
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

    if (next) return sem_struct_access_helper(next, type);
    return type;
}

Type_info* sem_struct_access(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_STRUCT) return NULL;

    char* str = tree->value;
    
    Symbol* symbol = check_for_a_symbol(tree->value);
    if (!symbol) sem_panic("undefined variable in struct access");
    if (symbol->type->category != TC_STRUCT) return NULL;

    tree = tree->left_child;
    if (!tree) return NULL;
    return sem_struct_access_helper(tree, symbol->type);

}

int sem_if(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type == AST_IF_ELSE) {
        if (!tree->left_child || !tree->right_child) sem_panic_parser_error();
        if (!sem_if(tree->left_child)) sem_panic("invalid if statement");
        if (!sem_if(tree->right_child)) sem_panic("invalid if statement");
        return 1;
    } else if (tree->type == AST_IF) {
        if (!tree->left_child) sem_panic_parser_error();

        Type_info* type = sem_exp(tree->left_child);
        if (!type) sem_panic("invalid expression in if control");

        if (!type_compat(type, BASIC_BOOL))
            sem_panic("expression in if control statement must be boolean");
        
        if (!tree->right_child) sem_panic_parser_error();
        if (!sem_statements(tree->right_child)) sem_panic("invalid statement in if body");
        return 1;
    } else if (tree->type == AST_ELSE) {
        if (sem_if(tree->left_child)) return 1;
        if (!sem_statements(tree->left_child)) sem_panic("invalid statement in else body");
    }
    return 0;

}

int sem_inc_dec(AST_node* tree) {
    if (!tree) return 0;

    if(tree->type != AST_INC && tree->type != AST_DEC) return 0;
    if(!tree->left_child) sem_panic_parser_error();

    tree = tree->left_child;
    Symbol* symbol = check_for_a_symbol(tree->value);
    if (!symbol) sem_panic("undefined variable");

    if(!type_compat(symbol->type, BASIC_INT)) sem_panic("only integer values can be incremented/decremented");
    
    return 1;
}
    

int sem_for_control_helper(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type != AST_SEQ) return 0;

    if (!tree->left_child) return 0;
    if (!tree->right_child) return 0;
    AST_node* init = tree->left_child;

    Type_info* init_type = sem_place(init);
    if (!init_type) return 0;
    
    if (!type_compat(init_type, BASIC_INT))
        sem_panic("non integer expression in the first statment of for control statement");
    
    tree = tree->right_child;
    if (tree->type != AST_SEQ) return 0;

    AST_node* cond = tree->left_child;

    Type_info* cond_type = sem_exp(cond);
    if (!type_compat(cond_type, BASIC_BOOL))
        sem_panic("the expression in the second statement of for control, must be boolean");
    
    AST_node* inc_dec = tree->right_child;
    if(!sem_inc_dec(inc_dec)) sem_panic("invalid incrementation/decrementation");

    return 1;

}
int sem_for_control(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type != AST_FOR_CONTROL) return 0;
    if (!tree->left_child) sem_panic_parser_error();
    tree = tree->left_child;


    if (tree->type == AST_WHILE) {
        if (!tree->left_child) sem_panic_parser_error();
        tree = tree->left_child;
        
        Type_info* type = sem_exp(tree);
        if (!type) sem_panic("invalid expression in a for control statement");
        if (type->category != TC_BASIC) sem_panic("invalid expression in a for control statement");
        if (type->data.basic != BASIC_BOOL) sem_panic("the expression in a for control statement must be boolean");
        return 1;
    } else if (tree->type == AST_FOR) {
        if (!tree->left_child) sem_panic_parser_error();
        tree = tree->left_child;
        if(sem_for_control_helper(tree)) return 1;
        return 0;
    } else sem_panic("invalid for control");
    
    return 0;
}

int sem_for(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type != AST_FOR) return 0;
    
    int statements =  sem_statements(tree->right_child);
    return statements;
}

int sem_func_call_params(AST_node* tree, Type_info** params, int nparams) {
    if (!tree || !params) return 0;

    for (int i = 0; i < nparams; i++) {
        if (!tree || !tree->left_child) sem_panic("too few arguments in procedure call");
        if (tree->type != AST_SEQ) return 0;

        Type_info* type = sem_exp(tree->left_child);
        if (!type) sem_panic("invalid expression in function call");
        
        if (type_check(type, params[i]) != 1)
            sem_panic("argument of incompatible type is passed to the procedure call"); 

        tree = tree->right_child;
    }
    return 1;
}
Type_info* sem_func_call(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_PROC) return NULL;

    Symbol *symbol = check_for_a_symbol(tree->value);
    if (!symbol) sem_panic("undefined procedure called");

    if (symbol->type->category != TC_FUNCTION) sem_panic("called variable is not a procedure");
    if (sem_func_call_params(tree->left_child,
                symbol->type->data.function.params,
                symbol->type->data.function.no_params) != 1) sem_panic("invalid parameter in procedure call");
    return symbol->type->data.function.ret_type;
}
int sem_ret(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type != AST_RET) return 0;
    Type_info* type = NULL;

    if (tree->left_child) {
        tree = tree->left_child;
        type = sem_exp(tree);
        if (!type) sem_panic("invalid expression in return value");
    } else {
        type = get_basic_type(BASIC_VOID);
    }

    Symbol* symbol = current_proc->gt_entry_symbol;
    if(type_check(symbol->type->data.function.ret_type, type) != 1) 
        sem_panic("incompatible return type");
    return 1;
}
Type_info* sem_array_def(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type != AST_ARRAY) return NULL;

    int range = 0;
    if (tree->left_child) {
        if (tree->left_child->type != AST_ARRAY_RANGE) sem_panic_parser_error();
        range = atoi(tree->left_child->value);
    }
    Symbol* var = sem_var_def(tree->right_child);
    if (!var) sem_panic_parser_error();
   
    Type_info* array = create_array_type(var->name, var->type, range);
    if (!array) sem_panic_allocation();

    if (!insert_entry(current_proc->st, var->name, array)) sem_panic_allocation();

    return array;
}
void sem_combs(AST_node* tree) {
    if (!tree) return; 

    if (tree->type != AST_COMBS) return;

    sem_top_def(tree->left_child);
    sem_procs(tree->right_child);
}
void sem_program(AST_node* tree) {
    if (!tree) return; 

    if (tree->type != AST_PROGRAM) return;
    
    if (!tree->right_child) return;
    sem_combs(tree->right_child);

}
void print_type(Type_info* ti) {
    if (!ti) return;

    printf("===");
    if (ti->name) printf("%s", ti->name);
    printf("===\n");

    printf("cat: %d\n", ti->category);
    printf("size: %d\n", ti->size_byte);
    printf("align: %d\n", ti->alignment);
 
    switch(ti->category){
        case TC_BASIC:
            printf("basic\n");
            break;
        case TC_POINTER:
            printf("underlying datatype:\n");
            print_type(ti->data.pointer.pointed_to);
            break;
        case TC_ARRAY:
            printf("number of elems: %d\n", ti->data.array.no_elements);
            printf("type of elems:\n");
            print_type(ti->data.array.element_type);
            break;
        case TC_STRUCT:
            printf("number of fields: %d\n", ti->data.structure.no_fields);
            printf("type of fields:\n");
            for (int i = 0; i < ti->data.structure.no_fields; i++) {
                if (ti->data.structure.fields[i]->name)
                    printf("%s", ti->data.structure.fields[i]->name);
                print_type(ti->data.structure.fields[i]->type);
                printf("offset: %d\n", ti->data.structure.fields[i]->offset);
            }
            break;
        case TC_FUNCTION:
            if (ti->data.function.name) printf("%s", ti->data.function.name);
            printf("number of params: %d\n", ti->data.function.no_params);
            printf("return value: \n");
            print_type(ti->data.function.ret_type);
            for (int i = 0; i < ti->data.function.no_params; i++)
                print_type(ti->data.function.params[i]);
            break;
        default:
            return;
    }

}
void semantics(AST_node* tree) {
    if (!tree) return; 
    global_symbol_table = create_table();
    sem_program(tree);
}
    
