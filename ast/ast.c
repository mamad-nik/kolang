#include <string.h>
#include <stdlib.h>
#include"ast.h"

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
AST_node* init_tree(AST_type type, char* value, AST_node* left_child, AST_node* right_child) {
    AST_node* node = malloc(sizeof(AST_node));
    if (node == NULL) return NULL;
    char* tmp = strdup(value);
    if (tmp == NULL) {
        free(node);
        return NULL;
    }
    node->type = type;
    node->value = tmp;
    node->left_child =  left_child;
    node->right_child =  right_child;
    node->attr = BASIC_VOID;
    node->symbols = NULL;
    return node;
}

