#include"parser.h"
AST_node* parse_argument();
AST_node* parse_deref();
AST_node* parse_array_access();
AST_node* parse_place();
AST_node* parse_assignment();
AST_node* parse_lib_acc();
AST_node* parse_struct_access();
AST_node* parse_func_call();
//TODO
AST_node* parse_arith_exp();
AST_node* parse_all_val();


Token** tokens_p;


int switch_val(keywords id) {
    if (id == INTEGER_VAL || id == FLOAT_VAL || 
            id == BOOL_VAL || id == STR_VAL || id == BYTE_VAL) return 1;
    return 0;
}
int switch_type(keywords id) {
    if (id == INT || id == INT || id == BOOL || 
            id == STRING || id == FLOAT || id == BYTE) return 1;
    return 0;
}
void parser_panic(const char* msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

AST_type map_type(keywords id) {
    if (id == INT) return AST_INT;
    if (id == BOOL) return AST_BOOL;
    if (id == STRING) return AST_STRING;
    if (id == FLOAT) return AST_FLOAT;
    if (id == BYTE) return AST_BYTE;
    return AST_ERR;
}
AST_type map_op(keywords id) {
    if (id == PLUS) return AST_PLUS;
    if (id == MINUS) return AST_MINUS;
    return AST_ERR;
}
AST_node *parse_deref_symbol_helper(AST_node *tmp) {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    tmp = parse_struct_access();
    if(tmp) return tmp;

    tmp = parse_func_call();
    if(tmp) return tmp;

    tmp = parse_array_access();
    if(tmp) return tmp;
    
    if ((*tokens_p)->id == SYMBOL) {
        tmp = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL); 
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        return tmp;
    }
    return NULL;
}
AST_node *parse_deref_symbol() {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    
    AST_node* lib = parse_lib_acc();

    AST_node *tmp = NULL;
    tmp = parse_deref_symbol_helper(tmp);
    if (!tmp) return NULL;

    if (lib) { 
        lib->right_child = tmp;
        return lib;
    }
    return tmp;
}
//TODO: chaining
AST_node* parse_deref_helper() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id == VALUE || (*tokens_p)->id == OPEN_PAR) {
        int par = 0;
        if ((*tokens_p)->id == OPEN_PAR) par++;
        tokens_p++;
        AST_node* symbol = parse_deref_helper();
        if (!symbol) return NULL;

        if (par) {
            if ((*tokens_p)->id == CLOSE_PAR) tokens_p++;
            else parser_panic("uneven paranthesis");
        }

        AST_node* value = init_tree(AST_VALUE, "", symbol,  NULL);
        if (!value) parser_panic("error tree creation");
        return value; 

    }
    AST_node* symbol = parse_deref_symbol();
    if (!symbol) return NULL;
    tokens_p++;
    return symbol;
}
AST_node* parse_deref() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id == VALUE || 
            ((*tokens_p)->id == OPEN_PAR && tokens_p[1]->id == VALUE)) {
        return parse_deref_helper();
    }
    return NULL;
}
                

AST_node* parse_array_lit() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    AST_node* node = parse_all_val();
    if(!node) return NULL;

    node = init_tree(AST_ARRAY_LIT, "", node, NULL);
    if (!node) parser_panic("error tree creation");

    return node;
}
AST_node* parse_array_value() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id != OPEN_BRAC) return NULL;

    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    do {
        tmp = parse_array_lit();
        if (!tmp) parser_panic("error tree creation");
        if (tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            tokens_p++;
        }
    } while (tmp);
    if ((*tokens_p)->id == CLOSE_BRAC)
        parser_panic("closing bracket expected");
    tokens_p++;
    return par;
}
AST_node* parse_array_def() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id != OPEN_BRAC) return NULL;
    tokens_p++;

    if ((*tokens_p)->id == INT_VAL) {
        tmp = init_tree(AST_ARRAY_RANGE, (*tokens_p)->str, tmp, NULL); 
    }

    
    if ((*tokens_p)->id != CLOSE_BRAC) return NULL;

}
AST_node* parse_struct_lit() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    Token** tmp = tokens_p;
    AST_node* node = parse_assignment();
    if (node) return node;
    tokens_p = tmp;

    node = parse_all_val();
    if(node) return node;

    tokens_p = tmp;
    return NULL;
}

AST_node* parse_struct_value() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id != OPEN_CURL) return NULL;
    tokens_p++;

    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    do {
        tmp = parse_struct_lit();
        tmp = init_tree(AST_STRUCT_FIELD, "", tmp, NULL);
        if (!tmp) parser_panic("error tree creation");
        if (tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            tokens_p++;
        }
    } while (tmp);
    if ((*tokens_p)->id == CLOSE_CURL)
        parser_panic("closing curly bracket expected");
    tokens_p++;
    AST_node* value = init_tree(AST_VALUE, "", par, NULL);
    if (!value) parser_panic("error tree creation");
    return value;
}

AST_node* parse_func_call() {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id != SYMBOL && tokens_p[1]->id != OPEN_PAR) return NULL;
    AST_node* func = init_tree(AST_PROC, (*tokens_p)->str, NULL, NULL);
    if (!func) parser_panic("error tree creation");
    tokens_p += 2;
    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    do {
        tmp = parse_all_val();
        if (tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            if ((*tokens_p)->id != COMMA) parser_panic("error in function arguments");
            tokens_p++;
        }
    } while (tmp);
    if ((*tokens_p)->id == CLOSE_PAR) parser_panic("uneven paranthesis");
    func->left_child = par;
    tokens_p++;
    return func;
}

AST_node* parse_lib_acc() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id != SYMBOL || tokens_p[1]->id != COL) return NULL;
    AST_node* lib = init_tree(AST_LIB, (*tokens_p)->str, NULL, NULL);    
    if (!lib) parser_panic("error tree creation");
    tokens_p += 2;
    return lib;
}
    
AST_node* parse_struct_access() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOF) return NULL;
    
    if ((*tokens_p)->id != SYMBOL || tokens_p[1]->id != DOT) return NULL;
    char *str = (*tokens_p)->str;
    if (tokens_p[2]->id != SYMBOL) return NULL;
    AST_node* child = init_tree(AST_STRUCT_FIELD, (*tokens_p)->str, NULL, NULL);
    if (!child) parser_panic("error tree creation");
    AST_node* par = init_tree(AST_STRUCT , str, child, NULL);
    if (!par) parser_panic("error tree creation");
    tokens_p += 3;
    return par;
}
AST_node* parse_array_access() {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id != SYMBOL && tokens_p[1]->id != OPEN_BRAC) return NULL;
    AST_node* array_access = init_tree(AST_ARRAY_ACCESS, (*tokens_p)->str, NULL, NULL);
    tokens_p += 2;
    AST_node* tmp = parse_all_val();
    if (!tmp) parser_panic("invalid array index");
    array_access->left_child = tmp;
    if (tokens_p[1]->id != CLOSE_BRAC) return NULL;
    tokens_p += 2;
    return array_access;
}
AST_node *parse_swo_helper(AST_node *tmp) {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    tmp = parse_struct_access();
    if(tmp) return tmp;

    tmp = parse_func_call();
    if(tmp) return tmp;

    tmp = parse_deref();
    if(tmp) return tmp;

    tmp = parse_array_access();
    if(tmp) return tmp;
    
    if ((*tokens_p)->id == SYMBOL) {
        tmp = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL); 
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        return tmp;
    }
    return NULL;
}
AST_node *parse_swo() {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    
    AST_node* lib = parse_lib_acc();

    AST_node *tmp = NULL;
    tmp = parse_swo_helper(tmp);
    if (!tmp) return NULL;

    if (lib) { 
        lib->right_child = tmp;
        return lib;
    }
    return tmp;
}
//arithmetic parser
AST_node *parse_primary() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOF) return NULL;

    if ((*tokens_p)->id == MINUS || (*tokens_p)->id == PLUS) {
        keywords id = (*tokens_p)->id;
        tokens_p++;
        AST_node *lhs = parse_primary();
        if (lhs == NULL) return NULL;
        AST_node* tmp = init_tree(map_op(id), "", lhs, NULL);
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        return tmp;
    }

    if ((*tokens_p)->id == OPEN_PAR)  {
        tokens_p++;
        AST_node* tmp = parse_arith_exp();
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        if ((*tokens_p)->id == CLOSE_PAR) parser_panic("uneven paranthesis");
        tokens_p++;
        return tmp;
    }
    
    if ((*tokens_p)->id == INTEGER_VAL || (*tokens_p)->id == FLOAT_VAL) {
        AST_node* tmp = init_tree(map_type((*tokens_p)->id),(*tokens_p)->str, NULL, NULL);
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        return tmp;
    }
        
    AST_node* node = parse_swo();
    if (node) return node;
    
    return NULL;
}

AST_node* parse_assignment() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    AST_node* lhs = parse_place();
    if (!lhs || tokens_p[1]->id != ASSIGN) return NULL;
    tokens_p++;

    AST_node* rhs = parse_all_val();
    if (!rhs) return NULL;
    
    AST_node* assign = init_tree(AST_ASSIGN, "", lhs, rhs);
    if (!assign) parser_panic("error tree creation");
    tokens_p++;

    return assign; 
}
//LOAD
//XXX
AST_node* parse_load() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    if((*tokens_p)->id != LOAD) return NULL;
    tokens_p++;
    if((*tokens_p)->id != OPEN_PAR) parser_panic("invalid load");
    tokens_p++;
    if((*tokens_p)->id != STR_VAL) parser_panic("invalid load");
    AST_node* load_node = init_tree(AST_LOAD, (*tokens_p)->str, NULL, NULL);   
    if (!load_node) parser_panic("memory allocation in load");
    tokens_p++;
    if((*tokens_p)->id != CLOSE_PAR) parser_panic("invalid load");
    tokens_p++;
    return load_node;
}
//XXX
AST_node* parse_imports() {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    do {
        tmp = parse_load();
        if(tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL); 
            if (!tmp) parser_panic("memory allocation in loads");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            tokens_p++;
        }
    }while(tmp);
    return par;
}
//XXX
AST_node* parse_struct_def() {
    if (tokens_p == NULL ||  (*tokens_p)->id == EOF) return NULL;
    if ((*tokens_p)->id != STRUCT && tokens_p[1]->id != OPEN_CURL) return NULL;
    AST_node* struct_node = init_tree(AST_STRUCT, (*tokens_p)->str, NULL, NULL);  
    if((*tokens_p)->id != OPEN_CURL) parser_panic("struct_def violation");
    tokens_p += 2;
    AST_node* par = NULL; 
    AST_node* last = NULL;
    AST_node* tmp = NULL;
    do {
        tmp = parse_argument();
        if(tmp != NULL){
            tmp->type = AST_STRUCT_FIELD;
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (tmp) parser_panic("error tree creation");            
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            tokens_p++;
        }
    } while(tmp);
    if((*tokens_p)->id != CLOSE_CURL) parser_panic("struct_def violation");
    tokens_p++;
    struct_node->left_child = par;
    return struct_node;
}

AST_node* parse_argument_types() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    if (switch_type((*tokens_p)->id)) {
        AST_node* type_tree =
            init_tree(map_type((*tokens_p)->id), (*tokens_p)->str, NULL, NULL);
        if (type_tree == NULL) parser_panic("error tree creation"); 
        tokens_p++;
        return type_tree;
    }
    if ((*tokens_p)->id != SYMBOL) {
        char* str = (*tokens_p)->str;
        tokens_p++;
        AST_node* symbol_tree = parse_struct_def();
        if (symbol_tree) return symbol_tree;
        symbol_tree = init_tree(AST_SYMBOL, str , NULL, NULL);
        if (symbol_tree == NULL) return NULL;
        tokens_p++;
        return symbol_tree;
    }
   return NULL; 
}

AST_node* parse_argument_pointer() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOF) return NULL;

    AST_node* head_pointer = NULL;
    AST_node* last_pointer = NULL;
    while ((*tokens_p)->id == POINTER) {
        AST_node* pointer_tree = init_tree(AST_POINTER, (*tokens_p)->str, NULL, NULL);
        if (pointer_tree == NULL) parser_panic("init tree error");
        if (!head_pointer) head_pointer = pointer_tree;
        
        if (!last_pointer) last_pointer = pointer_tree;
        else {
            last_pointer->left_child = pointer_tree;
            last_pointer = last_pointer->left_child;
        }
        tokens_p++;
    }
    AST_node* types = parse_argument_types();
    if (types == NULL) parser_panic("init tree error");
    if (head_pointer) {
        last_pointer->left_child = types;
        types = head_pointer;
    }
    return types;
}
AST_node* parse_argument() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    
    if ((*tokens_p)->id != SYMBOL) return NULL;
    AST_node* symbol_tree = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL);
    if (symbol_tree == NULL) parser_panic("init tree error");
    tokens_p++;

    AST_node* pointer_tree = parse_argument_pointer();
    if (pointer_tree == NULL) parser_panic("init tree error");
    
    AST_node* tmp_node = init_tree(AST_TBF, "", symbol_tree, pointer_tree);
    if (tmp_node == NULL) parser_panic("init tree error");
    tokens_p++;
    return tmp_node;
}
// intentionaly avoids checking for a comma in the arguments, trying to make new line work
AST_node* parse_type_def() { 
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    if((*tokens_p)->id != TYPE) return NULL;
    char* value = (*tokens_p)->str;
    tokens_p++;

    AST_node* arg_node = parse_argument();
    if (arg_node == NULL) return NULL;
    arg_node->type = AST_TYPE;
    arg_node->value = value;
    return arg_node;
}

AST_node* parse_var_def() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    if ((*tokens_p)->id != VAR) return NULL;
    char* str = (*tokens_p)->str;
    tokens_p++;
    AST_node* var_node = parse_argument();
    if(!var_node) return NULL;
    var_node->type = AST_VAR;
    var_node->value = str;
    return var_node;
}
AST_node* parse_place() {
    if (tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;

    Token** tmp = tokens_p;
    AST_node* node = parse_var_def();
    if (node) return node;
    tokens_p = tmp;

    node = parse_struct_access();
    if (node) return node;
    tokens_p = tmp;

    node = parse_array_access();
    if (node) return node;
    tokens_p = tmp;

    node = parse_deref();
    if (node) return node;
    tokens_p = tmp;

    if ((*tokens_p)->id == SYMBOL) {
        node = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL);
        tokens_p++;
        return node;
    }
    return NULL;
}

//TOP_DEF
AST_node* parse_top_def() {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    AST_node* tmp = parse_var_def();
    if(tmp) return tmp;
    return parse_type_def();
}
//since top defs are not mandatory, this function always returns null
AST_node* parse_top_defs() {
    if(tokens_p == NULL || (*tokens_p)->id == EOF) return NULL;
    AST_node* par = NULL; 
    AST_node* last = NULL;
    AST_node* tmp = NULL;
    do {
        tmp = parse_top_def();
        if(tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (tmp) parser_panic("error tree creation");            
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            tokens_p++;
        }
    }while(tmp);
    return par;
}
/*

Token** parse_array_access(Token** tokens) {
    if (tokens == NULL) return NULL;

    if ((*tokens)->id != SYMBOL) return NULL;
    tokens++;
    if ((*tokens)->id != OPEN_BRAC) return NULL;
    tokens++;
    if ((*tokens)->id != INTEGER_VAL) return NULL;
    tokens++;
    if ((*tokens)->id != CLOSE_BRAC) return NULL;
    tokens++;
    return tokens;
}

Token** parse_struct_access(Token** tokens) {
    if (tokens == NULL) return NULL;
    
    if ((*tokens)->id != SYMBOL) return NULL;
    tokens++;
    if ((*tokens)->id != DOT) return NULL;
    tokens++;
    if ((*tokens)->id != SYMBOL) return NULL;
    tokens++;
    return tokens;
}
Token** parse_if(Token** tokens) {
    if (tokens == NULL) return NULL;

    if ((*tokens)->id != IF) return NULL;
    tokens++;
    Token** tmp = parse_conditional(tokens);
    if (tmp == NULL) return NULL;
    tokens = tmp;
    if ((*tokens)->id != OPEN_CURL) return NULL;
    tokens++;
    tmp = NULL;
    do {
        tmp = parse_statements(tokens);
        if (tmp) tokens++;
    } while(tmp);
    if ((*tokens)->id != CLOSE_CURL) return NULL;
    tokens++;
    return tokens;
    
}
Token** parse_for(Token** tokens) {
    if (tokens == NULL) return NULL;

    if ((*tokens)->id != FOR) return NULL;
    tokens++;
    Token** tmp = tokens;
    tmp = parse_var_def(tmp);
    if (tmp != NULL) {
        tokens = tmp;
        if((*tokens)->id != SEMI_COL) return NULL;
        tokens++;
    }
    tokens = parse_conditionals(tokens);
    if (tokens == NULL) return NULL;
    if (tmp != NULL) {
        if ((*tokens)->id != SEMI_COL) return NULL;
        tokens++;
        if ((*tokens)->id != SYMBOL) return NULL;
        tokens++;
        tokens = parse_inc_dec(tokens);
        if (tokens == NULL) return NULL;
    }
    if ((*tokens)->id != OPEN_CURL) return NULL;
    tokens++;
    tokens = parse_statements(tokens);
    if (tokens == NULL) return NULL;
    if ((*tokens)->id != CLOSE_CURL) return NULL;
    tokens++;
    return tokens
}

Token** parse_statements(Token** tokens) {
    if (tokens == NULL) return NULL;

    if (parse_var_def(tokens)) return tokens;
    if (parse_if(tokens)) return tokens;
    if (parse_for(tokens)) return tokens;
    if (parse_func_call(tokens)) return tokens;
    return NULL;
}
int parse_return(Token** tokens) {
    if (tokens == NULL) return 0;
    if ((*tokens)->id != RET) return 0;
    tokens++;
    //TODO: implement all_val parser
    //if ((*tokens

    return 1;
}
Token** parse_inc_dec(Token** tokens) {
    if (tokens == NULL) return 0;
   Token** t = tokens;
    if ((*t)->id == PLUS) {
        if ((*t[1])->id == PLUS) {
            tokens = t[2];
            return tokens;
        }
    }
    t = tokens;
    if ((*t)->id == MINUS) {
        t++;
        if ((*t[1])->id == MINUS) { 
            tokens = t[2];
            return tokens;
        }
    }
    return NULL;
}


int parse_proc(Token** tokens)  {
    if (tokens == NULL) return 0;
    if ((*tokens)->id != PROC) return 0;
    tokens++;
    if ((*tokens)->id != SYMBOL) return 0;
    tokens++;
    if ((*tokens)->id != OPEN_PAR) return 0;
    int no_arguments = 0;
    do {
        tokens++;
    } while(no_arguments);
    if ((*tokens)->id != CLOSE_PAR) return 0;
    tokens++;



}  
*/
void print_tree(AST_node* node) {
    if (node == NULL) return;
    printf("%d, ", node->type);
    if (node->value != NULL) printf("%s\n", node->value);
    print_tree(node->left_child);
    print_tree(node->right_child);
}
    
int parse(Stream* input_stream) {
    if (input_stream == NULL) return 0;
    tokens_p = input_stream->tokens;
    AST_node* node = parse_imports();
    print_tree(node);
    return 1;
}
