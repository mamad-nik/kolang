#include <string.h>
#include <stdlib.h>
#include"ast.h"

AST_node* init_tree(AST_type type, char* value, AST_node* left_child, AST_node* right_child) {
    AST_node* node = malloc(sizeof(AST_node));
    if (node == NULL) return NULL;
    char* tmp = strdup(value);
    if (tmp == NULL) return NULL;
    node->type = type;
    node->value = tmp;
    node->left_child =  left_child;
    node->right_child =  right_child;
    return node;
}

