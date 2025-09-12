#include"ircodegen.h"

CG* cg;

//TODO
char* icg_exp(AST_node* tree);
char* icg_statements(AST_node* tree);
int icg_assignment(AST_node* tree);
int icg_inc_dec(AST_node* tree);

char* new_label() {
    char* label = malloc(32);
    snprintf(label, 32, "@L%d", cg->label_counter++);
    return label;
}
char* new_temp() {
    char* temp = malloc(32);
    snprintf(temp, 32, "%%s%d", cg->label_counter++);
    return temp;
}

int icg_switch_op(AST_type op)  {
    if (op == AST_PLUS || op == AST_MINUS || op == AST_TIMES ||
            op == AST_DIVIDE || op == AST_MODULO || op == AST_AND ||
            op == AST_OR || op == AST_EQ || op == AST_NEQ || op == AST_LT 
            || op == AST_GT || op == AST_LE || op == AST_GE || op == AST_EX_MARK) return 1; 
    return 0;
}

char* map_basic_qbe(Basic_type basic) {
    switch(basic) {
        case BASIC_INT:
            return "l";
            break;
        case BASIC_FLOAT:
            return "d";
            break;
        case BASIC_BOOL:
            return "w";
            break;
        case BASIC_BYTE:
            return "w";
            break;
        case BASIC_STRING:
            return "l";
            break;
        default: 
            return NULL;
            break;
    }
    return NULL;
}


char* map_type_qbe(Type_info* ti) {
    if (!ti) return NULL;

    switch (ti->category) {
        case TC_BASIC:
            return map_basic_qbe(ti->data.basic);
            break;
        case TC_POINTER:
        case TC_ARRAY:
        case TC_STRUCT:
            return "l";
        default:
            return NULL;
    }
    return NULL;
}


int get_align_basic(Basic_type basic) {
    switch(basic) {
        case BASIC_BOOL:
        case BASIC_BYTE:
            return 4;
            break;
        case BASIC_INT:
        case BASIC_FLOAT:
        case BASIC_STRING:
            return 8;
            break;
        default: 
            return 0;
            break;
    }
    return 0;
}

int get_align(Type_info* ti) {
    if (!ti) return 0;

    switch (ti->category) {
        case TC_BASIC:
            return get_basic_type_align(ti->data.basic);
            break;
        case TC_POINTER:
        case TC_ARRAY:
        case TC_STRUCT:
            return 8; 
        default:
            return 0;
    }
    return 0;
}

char* var_def(AST_node* tree) {
    if (!tree) return NULL;
    
    Symbol* symbol = 
        icg_lookup_symbol(cg, tree->left_child->value);
    if (!symbol) return NULL;
    char* loc = new_temp();
    int align = get_align(symbol->type);

    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "%s =l alloc%d %d", loc, align, align);
    symbol->loc = loc;
    return loc;

}
void set_var(char* dst, char* src, char* type) {
    if (!dst || !src || !type) return;

    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "store%s %s, %s", type, src, dst);
}
char* get_var(char* name) {
    if (!name) return NULL;

    Symbol* symbol = icg_lookup_symbol(cg, name);
    if (!symbol) return NULL;

    char* type = map_type_qbe(symbol->type);
    char* temp = new_temp();

    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "%s =%s load%s %s", temp, type, type, name);
    return temp;
}

char* int_or_float(Basic_type type) {
    if (!type) return NULL;

    if (type == BASIC_FLOAT) return "";
    if (type == BASIC_INT) return "s";
    return NULL;
}

int icg_bool(char* str) {
    if (!str) return -1;
    if (strcmp(str, "true") == 0) return 1;
    if (strcmp(str, "false") == 0) return 0;
    return -1;
}

char* func_call(AST_node* tree) {
    if (!tree) return NULL;

    Symbol* symbol = icg_lookup_symbol(cg, tree->value);
    tree = tree->left_child;

    if (!symbol) return NULL;
    Type_info* ti = symbol->type;

    int no_params = ti->data.function.no_params;
    Type_info *ret = ti->data.function.ret_type;

    char* ret_val;
    char* tmp = "";
    if (ret->category == TC_BASIC && ret->data.basic == BASIC_VOID)
        ret_val = "";
    else {
        tmp = new_temp();
        ret_val = map_type_qbe(ret);
        ret_val = create_formatted_string("%s =%s ", tmp, ret_val);
    }

    char* params = "";
    
    int i = 0;
    while(tree) {
        if (tree->type != AST_SEQ) return 0;
        char* partype = map_type_qbe(ti->data.function.params[i]);
        if (!partype) return NULL;
        char* temp = icg_exp(tree->left_child);
        if (!temp) return NULL;
        params = create_formatted_string("%s%s%s %s, ",
                ret_val, params, partype, temp);
        tree = tree->right_child;
    }

    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "   call %s(%s)\n", symbol->name, params);
    return tmp;
}
char* icg_primary(AST_node* tree) {
    if (!tree) return NULL;
    switch (tree->type) {
        case(AST_BOOL_VAL): {
            int val = icg_bool(tree->value);
            char* res = new_temp();
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =w copy %d\n", res, val);
            return res;
        }
        case (AST_INTEGER_VAL): {
            int val = atoi(tree->value);
            char* res = new_temp();
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =l copy %d\n", res, val);
            return res;
        }
        case (AST_FLOAT_VAL): {
            int val = atof(tree->value);
            char* res = new_temp();
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =l copy %d\n", res, val);
            return res;

        }
        case (AST_SYMBOL): {
            char* val = get_var(tree->value);
            if (!val) return NULL;
            return val;
        }

    }
    return NULL;


}
char* icg_exp(AST_node* tree) {
    if (!tree) return NULL;

    if (tree->type == AST_EXPR) tree = tree->left_child;

    if (!icg_switch_op(tree->type)) return icg_primary(tree);

    switch (tree->type) {
        case AST_PLUS: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s add %s, %s\n", res, type, op1, op2);   
            return res;
        }
        case AST_MINUS: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s sub %s, %s\n", res, type, op1, op2);   
            return res;
        }
        case AST_TIMES: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s mul %s, %s\n", res, type, op1, op2);   
            return res;
        }
        case AST_DIVIDE: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s div %s, %s\n", res, type, op1, op2);   
            return res;
        }
        case AST_MODULO: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s rem %s, %s\n", res, type, op1, op2);   
            return res;
        }
        case AST_EQ: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s ceq%s %s, %s\n", res, type, type, op1, op2);   
            return res;
         }
        case AST_NEQ: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s cneq%s %s, %s\n", res, type, type, op1, op2);   
            return res;
        }
        case AST_GT: {
            char* prefix = int_or_float(tree->attr);
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s c%sgt%s %s, %s\n", res, type, prefix, type, op1, op2);   
            return res;
        }
        case AST_GE: {
            char* prefix = int_or_float(tree->attr);
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s c%sge%s %s, %s\n", res, type, prefix, type, op1, op2);   
            return res;
        }
        case AST_LT: {
            char* prefix = int_or_float(tree->attr);
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s c%slt%s %s, %s\n", res, type, prefix, type, op1, op2);   
            return res;
        }
        case AST_LE: {
            char* prefix = int_or_float(tree->attr);
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            char* type = map_basic_qbe(tree->attr);
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =%s c%sle%s %s, %s\n", res, type, prefix, type, op1, op2);   
            return res;
        }
        case AST_AND: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =w and %s, %s\n", res, op1, op2);
            return res;
        }
        case AST_OR: {
            char* op1 = icg_exp(tree->left_child);
            char* op2 = icg_exp(tree->right_child);
            char* res = new_temp();
            cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                    "   %s =w or %s, %s\n", res, op1, op2);
            return res;
        }
        default:
             return NULL;

    }
    return NULL;

}
char* icg_create_struct(char* name) {
    if (!name) return NULL;
    
    Type_info* type = get_type_str(name);
    if (!type) return NULL;

    int size = type->size_byte;
    char* temp = new_temp();
    
    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "%s =l call $malloc(l %d)", temp, size);
    
    return temp;
}
char* icg_set_struct_field(char* name, char* field) {
    if (!name || !field) return NULL;

    Symbol* symbol = icg_lookup_symbol(cg, name);
    if (!symbol) return NULL;
    char* loc = symbol->loc;

    Type_info* type = symbol->type;
    if (!type) return NULL;

    int offset;
    int no_fields = type->data.structure.no_fields;
    for (int i = 0; i < no_fields; i++) {
        Struct_field* strfield = type->data.structure.fields[i];
        if (strcmp(field, strfield) == 0) offset = strfield->offset;
    }
    char* temp1 = new_temp();
    char* temp2 = new_temp();
    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "   %s =l loadl %s\n" 
            "   %s =l add %s, %d\n", temp1, loc, temp2, temp1, offset); 
    
    return temp2;
}
void icg_for(AST_node* control, AST_node* statements) {
    if (!control || !statements) return;

    if (control->type != AST_FOR) return;
    control = control->left_child;
    if (control->type != AST_SEQ) return;

    if (!icg_assignment(control->left_child)) return;
    
    control = control->right_child;
    if (control->type != AST_SEQ) return;
    
    char* cond = new_label();
    char* body = new_label();
    char* step = new_label();
    char* end = new_label();
    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "%s\n", cond);
    char* res = icg_exp(control->left_child);
    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "   jnz %s, %s, %s\n"
            "%s\n", res, body, end, body);

    icg_statements(statements);
    
    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "%s\n", step);
    icg_inc_dec(control->left_child);

    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "   jmp %s\n"
            "%s\n", cond, end);
    
}
void icg_while(AST_node* control, AST_node* statements) {
    if (!control || !statements) return;

    if (control->type != AST_WHILE) return;

    char* label_string = icg_exp(control->left_child);
    char* forcontrol = new_label();
    char* forbody = new_label();
    char* forend = new_label();

    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "%s\n"
            "   jnz %s, %s, %s\n"
            "%s\n", forcontrol, control, forbody, forend, forbody);

    icg_statements(statements);
    cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
            "   jmp %s\n"
            "%s\n", forcontrol, forend);
    
}

int icg_loop(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type != AST_FOR) return 0;
    AST_node* control = tree->left_child;
    AST_node* statements = tree->right_child;

    Table* tmp = cg->curr_scope;
    cg->curr_scope = tree->symbols;

    if(control->type != AST_FOR_CONTROL) return 0;
    control = control->left_child;
    
    if (control->type == AST_WHILE) icg_while(control, statements);
    else if (control->type == AST_FOR) icg_for(control, statements);

    cg->curr_scope = tmp; 
    return 1;

}

int icg_if(AST_node* tree) {
    if (!tree) return 0;

    if (tree->type == AST_IF_ELSE) {
        icg_if(tree->left_child);
        icg_if(tree->right_child);
        return 1;
    } else if (tree->type == AST_IF) {
        Table* tmp = cg->curr_scope;
        cg->curr_scope = tree->symbols;
        char* control = icg_exp(tree->left_child);
        char* ifthen = new_label();
        char* ifelse = new_label();

        cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                "   jnz %s, %s, %s\n"
                "%s\n", control, ifthen, ifelse, ifthen);

        icg_statements(tree->right_child);

        cg->curr_proc->buffer = cb_append(cg->curr_proc->buffer, 
                "%s\n", ifelse);
        cg->curr_scope = tmp;
        return 1;
    } else if (tree->type == AST_ELSE) {
        if (icg_if(tree->left_child)) return 1;

        Table* tmp = cg->curr_scope;
        cg->curr_scope = tree->symbols;

        icg_statements(tree->left_child);

        cg->curr_scope = tmp;
        return 1;
    }
    return 0;
}


