#include"ircodegen.h"

CG* cg;

char* icg_exp(AST_node* tree);
char* icg_statements(AST_node* tree);

char* new_label() {
    char* label = malloc(32);
    snprintf(label, 32, "@L%d", cg->label_counter++);
    return label;
}

char*  map_basic_qbe(Basic_type basic) {
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
            return "l";  // pointers are longs in QBE
        default:
            return NULL;
    }
    return NULL;
}

int icg_for(AST_node* tree) {
    if (!tree) return 0;

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


