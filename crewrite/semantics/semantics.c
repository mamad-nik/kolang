#include"semantics.h"

typedef struct {
    char* name;
    Symbol* gt_entry_symbol;
    Table* st;
} Cproc;

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

    Entry* entry = lookup_entry(global_symbol_table, name);
    p->gt_entry_symbol = (Symbol*)entry->value; 
    p->st = table; 
    p->name = strdup(name);
    return p;
}

void sem_gvar_def(AST_node* tree) {
    if (!tree) return;

    if (tree->type != AST_VAR) return;
    char* str  = tree->left_child->value;
    char* type = tree->right_child->value; 

    Entry* entry = lookup_entry(global_symbol_table, str);
    if (!entry) return;
    Symbol* symbol = (Symbol*)entry->value; 

    symbol->type = type;
    symbol->size_bytes = map_type_size(type);
    symbol->is_func = 0;   
    symbol->scope = scope;
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
void sem_prec_out(AST_node* tree) {
    if (!tree) return;
    Symbol* symbol = current_proc->gt_entry_symbol;
    symbol->type = strdup(tree->value);
}
    
    
void sem_prec_inp_out(AST_node* tree) {
    if (!tree) return;

    scope = SCOPE_PARAMETER;
    if (tree->right_child) sem_prec_out(tree->right_child);
    if (!tree->left_child) return;

    AST_node* head = tree->left_child;
    while (head) {
        if (head->type == AST_SEQ) {
           AST_node* node = head->left_child;
            //create_symbol(node->value
            }
    }
}
    

void sem_proc(AST_node* tree) {
    if (!tree) return;

    if (tree->type != AST_PROC) return;
    scope = SCOPE_GLOBAL;
    char* str = tree->value;

    AST_node* statements = tree->right_child;
    AST_node* head = tree->left_child;

    Cproc* cproc = create_cproc(str);
    current_proc = cproc;

    symbol->is_func = 1;
    if (head->right_child) symbol->type = head->right_child->value;
    
    scope = SCOPE_LOCAL;
}

void sem_var_def(AST_node* tree) {
    if (!tree) return;

    if (tree->type != AST_VAR) return;

    char* str  = tree->left_child->value;
    char* type = tree->right_child->value; 

    Entry* entry = lookup_entry(symbol_table, str);
    if (!entry) return;
    Symbol* symbol = (Symbol*)entry->value; 

    symbol->type = type;
    symbol->is_func = 0;   
    symbol->size_bytes = map_type_size(type);
    symbol->scope = SCOPE_LOCAL;
}
void semantics(AST_node* tree, Table* symbol_table) {}
    
