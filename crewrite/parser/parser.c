#include"parser.h"
AST_node* parse_argument();
AST_node* parse_deref();
AST_node* parse_array_access();
AST_node* parse_place();
AST_node* parse_assignment();
AST_node* parse_lib_acc();
AST_node* parse_struct_access();
AST_node* parse_func_call();
AST_node* parse_var_def();
AST_node* parse_exp();
AST_node* parse_statements();
AST_node* parse_conditional();
AST_node* parse_if();


Token** tokens_p;

int switch_type(keywords id) {
    if (id == INT || id == BOOL || id == STRING
            || id == FLOAT || id == BYTE) return 1;
    return 0;
}
int switch_type_val(keywords id) {
    if (id == INTEGER_VAL || id == BOOL_VAL || id == STR_VAL
            || id == FLOAT_VAL || id == BYTE_VAL) return 1;
    return 0;
}
void parser_panic(const char* msg) {
    fprintf(stderr, "ERROR: %s approximately at %d\n", msg, (*tokens_p)->line_no);
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
AST_type map_type_val(keywords id) {
    if (id == INTEGER_VAL) return AST_INTEGER_VAL;
    if (id == BOOL_VAL) return AST_BOOL_VAL;
    if (id == STR_VAL) return AST_STR_VAL;
    if (id == FLOAT_VAL) return AST_FLOAT_VAL;
    if (id == BYTE_VAL) return AST_BYTE_VAL;
    return AST_ERR;
}
AST_type map_op(keywords id) {
    if (id == PLUS) return AST_PLUS;
    if (id == MINUS) return AST_MINUS;
    if (id == TIMES) return AST_TIMES;
    if (id == DIVIDE) return AST_DIVIDE;
    if (id == MODULO) return AST_MODULO;
    if (id == AND) return AST_AND; 
    if (id == OR) return AST_OR;
    if (id == NEQ) return AST_NEQ;
    if (id == EQ) return AST_EQ;
    if (id == GT) return AST_GT;
    if (id == LT) return AST_LT;
    if (id == GE) return AST_GE;
    if (id == LE) return AST_LE;
    return AST_ERR;
}
char* map_op_str(keywords id) {
    if (id == PLUS) return "+";
    if (id == MINUS) return "-";
    if (id == TIMES) return "*";
    if (id == DIVIDE) return "/";
    if (id == MODULO) return "%";
    if (id == AND) return "&&"; 
    if (id == OR) return "||";
    if (id == NEQ) return  "!=";
    if (id == EQ) return "==";
    if (id == GT) return ">";
    if (id == LT) return "<";
    if (id == GE) return ">=";
    if (id == LE) return "<=";
    return "";
}
AST_node *parse_deref_symbol_helper(AST_node *tmp) {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

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
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    
    AST_node* lib = parse_lib_acc();

    AST_node *tmp = NULL;
    tmp = parse_deref_symbol_helper(tmp);
    if (!tmp) return NULL;

    if (lib) { 
        lib->left_child = tmp;
        return lib;
    }
    return tmp;
}
AST_node* parse_deref_helper() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

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

        AST_node* value = init_tree(AST_VALUE, "value", symbol,  NULL);
        if (!value) parser_panic("error tree creation");
        return value; 

    }
    AST_node* symbol = parse_deref_symbol();
    if (!symbol) return NULL;
    return symbol;
}
AST_node* parse_deref() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id == VALUE || 
            ((*tokens_p)->id == OPEN_PAR && tokens_p[1]->id == VALUE)) {
        return parse_deref_helper();
    }
    return NULL;
}
                

AST_node* parse_array_lit() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* node = parse_exp();
    if(!node) return NULL;

    node = init_tree(AST_ARRAY_LIT, "", node, NULL);
    if (!node) parser_panic("error tree creation");

    return node;
}
AST_node* parse_array_value() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != OPEN_BRAC) return NULL;
    tokens_p++;

    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    int memnum = 0;
    int comnum = 0;
    do {
        tmp = parse_array_lit();
        if (tmp) {
            memnum++;
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            if ((*tokens_p)->id == COMMA) {
                comnum++;
                tokens_p++;
            }
        } else {
            if (memnum != comnum+1) parser_panic("commas are mandatory in array values");
        }
    } while (tmp);
    if ((*tokens_p)->id != CLOSE_BRAC)
        parser_panic("closing bracket expected");
    tokens_p++;
    AST_node* value = init_tree(AST_ARRAY_VALUE, "", par, NULL);
    if (!value) parser_panic("error tree creation");
    return par;
}

AST_node* parse_array_range() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* range = NULL;
    if ((*tokens_p)->id == INTEGER_VAL || (*tokens_p)->id == SYMBOL) {
        range = init_tree(AST_ARRAY_RANGE, (*tokens_p)->str, NULL, NULL); 
        tokens_p++;
    } 
    return range;
}

AST_node* parse_array_def() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* var = parse_var_def(); 
    if (!var) return NULL;


    if ((*tokens_p)->id != OPEN_BRAC) return NULL;
    tokens_p++;

    AST_node* range = parse_array_range();

    if ((*tokens_p)->id != CLOSE_BRAC) return NULL;
    tokens_p++;
    AST_node* tree = init_tree(AST_ARRAY, "", range, var); 
    if (!tree) parser_panic("error tree creation");
    return tree;
}

AST_node* parse_struct_lit() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    Token** tmp = tokens_p;
    AST_node* node = parse_assignment();
    if (node) return node;
    tokens_p = tmp;

    node = parse_exp();
    if(node) return node;

    tokens_p = tmp;
    return NULL;
}

AST_node* parse_struct_value() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != SYMBOL || tokens_p[1]->id != OPEN_CURL) return NULL;
    char* str = (*tokens_p)->str;
    tokens_p += 2;

    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    do {
        tmp = parse_struct_lit();
        if (tmp) {
            tmp = init_tree(AST_STRUCT_FIELD, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
        }
    } while (tmp);
    if ((*tokens_p)->id != CLOSE_CURL)
        parser_panic("closing curly bracket expected");
    tokens_p++;
    AST_node* value = init_tree(AST_STRUCT_VALUE, str, par, NULL);
    if (!value) parser_panic("error tree creation");
    return value;
}

AST_node* parse_func_call() {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != SYMBOL || tokens_p[1]->id != OPEN_PAR) return NULL;
    AST_node* func = init_tree(AST_PROC, (*tokens_p)->str, NULL, NULL);
    if (!func) parser_panic("error tree creation");
    tokens_p += 2;
    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    int memno = 0;
    int comno = 0;
    do {
        tmp = parse_exp();
        if (tmp) {
            memno++;
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
            if ((*tokens_p)->id == COMMA) {
                comno++;
                tokens_p++;
            }
        } else {
            if (memno != comno +1) parser_panic("commas are mandatory in procedure call");
        }
    } while (tmp);
    if ((*tokens_p)->id != CLOSE_PAR) parser_panic("uneven paranthesis");
    func->left_child = par;
    tokens_p++;
    return func;
}

AST_node* parse_lib_acc() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != SYMBOL || tokens_p[1]->id != COL) return NULL;
    AST_node* lib = init_tree(AST_LIB, (*tokens_p)->str, NULL, NULL);    
    if (!lib) parser_panic("error tree creation");
    tokens_p += 2;
    return lib;
}

AST_node* parse_struct_access_helper() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != DOT) return NULL;
    tokens_p++;
    
    Token** tokens = tokens_p;
    AST_node* tmp = parse_array_access();
    if (tmp) return tmp;
    tokens_p = tokens;

    if ((*tokens_p)->id == OPEN_PAR && tokens_p[1]->id == VALUE) {
        tokens_p++;
        tmp = parse_deref();
        if(!tmp) parser_panic("invalid struct access");
        if ((*tokens_p)->id != CLOSE_PAR) parser_panic("uneven paranthesis"); 
        tokens_p++;
        return tmp;
    }

    if ((*tokens_p)->id == SYMBOL) {
        tmp = init_tree(AST_TBF, (*tokens_p)->str, NULL, NULL);
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        return tmp;
    }
    return NULL;
}
    
AST_node* parse_struct_access_tag() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOFS) return NULL;

    AST_node* node = parse_struct_access_helper();
    if (!node) return NULL;

    if (node->type == AST_TBF) {
        node->type = AST_STRUCT_FIELD;
        return node;
    } 

    node = init_tree(AST_STRUCT_FIELD, "", node, NULL);
    if (!node) parser_panic("error tree creation"); 
    return node;
}
    
AST_node* parse_struct_access() {
    if(tokens_p == NULL ||  (*tokens_p)->id == EOFS) return NULL;
    
    if ((*tokens_p)->id != SYMBOL || tokens_p[1]->id != DOT) return NULL;
    tokens_p++;
    
    AST_node* tmp = NULL;
    AST_node* par = NULL;
    AST_node* last = NULL;
    do {
        tmp = parse_struct_access_tag();
        if(tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL); 
            if (!tmp) parser_panic("error tree creation");
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
        }
    }while(tmp);

    par = init_tree(AST_STRUCT, (*tokens_p)->str, par, NULL);
    if (!par) parser_panic("error tree creation");
    return par;
}
AST_node* parse_array_access() {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != SYMBOL || tokens_p[1]->id != OPEN_BRAC) return NULL;
    AST_node* array_access = init_tree(AST_ARRAY_ACCESS, (*tokens_p)->str, NULL, NULL);
    tokens_p += 2;
    AST_node* tmp = parse_exp();
    if (!tmp) parser_panic("invalid array index");
    array_access->left_child = tmp;
    if ((*tokens_p)->id != CLOSE_BRAC) parser_panic("closing bracket expected");
    tokens_p++;
    return array_access;
}
AST_node *parse_swo_helper(AST_node *tmp) {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    Token** tokens = tokens_p;

    tmp = parse_struct_access();
    if(tmp) return tmp;
    tokens_p = tokens;

    tmp = parse_func_call();
    if(tmp) return tmp;
    tokens_p = tokens;

    tmp = parse_deref();
    if(tmp) return tmp;
    tokens_p = tokens;

    tmp = parse_array_access();
    if(tmp) return tmp;
    tokens_p = tokens;

    tmp = parse_struct_value();
    if(tmp) return tmp;
    tokens_p = tokens;
    
    tmp = parse_array_value();
    if(tmp) return tmp;
    tokens_p = tokens;

    if ((*tokens_p)->id == SYMBOL) {
        tmp = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL); 
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        return tmp;
    }
    return NULL;
}
AST_node *parse_swo() {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    
    AST_node* lib = parse_lib_acc();

    AST_node *tmp = NULL;
    tmp = parse_swo_helper(tmp);
    if (!tmp) return NULL;

    if (lib) { 
        lib->left_child = tmp;
        return lib;
    }
    return tmp;
}
//arithmetic parser
typedef struct {
    keywords type;
    int prec;
} Prec;

const Prec precs[] = {
    {PLUS, 10},
    {MINUS, 10},
    {TIMES, 20}, 
    {DIVIDE, 20}, 
    {MODULO, 20},
    {OR, 1},
    {AND, 2},
    {NEQ, 5},
    {EQ, 5},
    {GT, 5},
    {LT, 5},
    {GE, 5},
    {LE, 5},
    {EOFS, 0}
};

int is_non_associative(keywords op) {
    if (op == EQ || op == NEQ || op == LT || op == GT || op == LE || op == GE) return 1; 
    return 0;
}

int arith_prec_check(keywords id) {
    for (int i = 0; i < sizeof(precs)/sizeof(precs[0]); i++) 
        if (precs[i].type == id) return precs[i].prec;
    return -1;
}

AST_node *parse_arith_primary() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id == MINUS ||
            (*tokens_p)->id == PLUS || (*tokens_p)->id == EX_MARK) {
        keywords id = (*tokens_p)->id;
        char* str = (*tokens_p)->str;
        tokens_p++;
        AST_node *lhs = parse_arith_primary();
        if (lhs == NULL) return NULL;
        AST_node* tmp = init_tree(map_op(id), str, lhs, NULL);
        if (!tmp) parser_panic("error tree creation");
        return tmp;
    }

    if ((*tokens_p)->id == OPEN_PAR)  {
        tokens_p++;
        AST_node* tmp = parse_exp();
        if (!tmp) parser_panic("error tree creation");
        if ((*tokens_p)->id != CLOSE_PAR) parser_panic("uneven paranthesis");
        tokens_p++;
        return tmp;
    }
    
    if (switch_type_val((*tokens_p)->id)) {
        AST_node* tmp = init_tree(map_type_val((*tokens_p)->id),
                (*tokens_p)->str, NULL, NULL);
        if (!tmp) parser_panic("error tree creation");
        tokens_p++;
        return tmp;
    }
        
    AST_node* node = parse_swo();
    if (node) return node;
    
    return NULL;
}
AST_node* arith_apply(keywords op, AST_node* lhs, AST_node* rhs) {
    AST_node* node = init_tree(map_op(op), map_op_str(op), lhs, rhs);
    if (!node) parser_panic("error tree creation");
    return node;
}
AST_node* parse_arith_exp_1(AST_node* lhs, int min_prec) {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    int prec = arith_prec_check((*tokens_p)->id);
    if (prec == -1) return NULL;
    
    while (prec > min_prec) {
        Token** tokens = tokens_p;
        keywords op = (*tokens_p)->id;
        tokens_p++;
        AST_node* rhs = parse_arith_primary();
        if (!rhs) {
            tokens_p = tokens;
            return lhs;
        }
        keywords nop = (*tokens_p)->id;
        int nprec = arith_prec_check(nop); 
        while (nprec >= prec) {
            rhs = parse_arith_exp_1(rhs, prec + (prec == nprec ? 1 : 0));
            nop = (*tokens_p)->id;
            nprec = arith_prec_check(nop); 
        }
        lhs = arith_apply(op, lhs, rhs);
        prec = arith_prec_check((*tokens_p)->id);

        if (is_non_associative(op)) {
            lhs = init_tree(AST_COND, "", lhs, NULL);
            if (!lhs) parser_panic("error tree creation");
            break;
        }
    }
    return lhs;
}
AST_node* parse_exp() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    AST_node* lhs = parse_arith_primary();
    if (!lhs) return NULL;
    AST_node* tmp = parse_arith_exp_1(lhs, 0);
    if(!tmp) {
        return lhs;
    }
    AST_node* node = init_tree(AST_EXPR, "", tmp, NULL);
    if(!node) parser_panic("error tree creation");
    return node;
}
AST_node* parse_assignment() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* lhs = parse_place();
    if (!lhs || (*tokens_p)->id != ASSIGN) return NULL;
    tokens_p++;

    AST_node* rhs = parse_exp();
    if (!rhs) parser_panic("invalid right-hand side statement");
    
    AST_node* assign = init_tree(AST_ASSIGN, "=", lhs, rhs);
    if (!assign) parser_panic("error tree creation");

    return assign; 
}
//LOAD
//XXX
AST_node* parse_load() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    if((*tokens_p)->id != LOAD) return NULL;
    tokens_p++;
    if((*tokens_p)->id != OPEN_PAR) parser_panic("expected paranthesis in load");
    tokens_p++;
    if((*tokens_p)->id != STR_VAL) parser_panic("invalid load");
    AST_node* load_node = init_tree(AST_LOAD, (*tokens_p)->str, NULL, NULL);   
    if (!load_node) parser_panic("memory allocation in load");
    tokens_p++;
    if((*tokens_p)->id != CLOSE_PAR) parser_panic("uneven paranthesis in load");
    tokens_p++;
    return load_node;
}
AST_node* parse_imports() {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
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
        }
    }while(tmp);
    return par;
}
AST_node* parse_struct_def() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    if ((*tokens_p)->id != STRUCT || tokens_p[1]->id != OPEN_CURL) return NULL;
    AST_node* struct_node = init_tree(AST_STRUCT, (*tokens_p)->str, NULL, NULL);  
    tokens_p += 2;
    AST_node* par = NULL; 
    AST_node* last = NULL;
    AST_node* tmp = NULL;
    do {
        tmp = parse_argument();
        if(tmp){
            tmp->type = AST_STRUCT_FIELD;
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");            
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
        }
    } while(tmp);
    if((*tokens_p)->id != CLOSE_CURL) parser_panic("uneven curly braces");
    tokens_p++;
    struct_node->left_child = par;
    return struct_node;
}

AST_node* parse_argument_types() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    if (switch_type((*tokens_p)->id)) {
        AST_node* type_tree =
            init_tree(map_type((*tokens_p)->id), (*tokens_p)->str, NULL, NULL);
        if (type_tree == NULL) parser_panic("error tree creation"); 
        tokens_p++;
        return type_tree;
    }
    AST_node* symbol_tree = parse_struct_def();
    if (symbol_tree) return symbol_tree;

    if ((*tokens_p)->id == SYMBOL) {
        symbol_tree = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL);
        if (symbol_tree == NULL) parser_panic("error tree creation");
        tokens_p++;
        return symbol_tree;
    }
   return NULL; 
}

AST_node* parse_argument_pointer() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

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
    if (types == NULL) parser_panic("error parsing argument types");
    if (head_pointer) {
        last_pointer->left_child = types;
        types = head_pointer;
    }
    return types;
}
AST_node* parse_argument() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    
    if ((*tokens_p)->id != SYMBOL) return NULL;
    AST_node* symbol_tree = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL);
    if (symbol_tree == NULL) parser_panic("error tree creation");
    tokens_p++;

    AST_node* pointer_tree = parse_argument_pointer();
    if (pointer_tree == NULL) parser_panic("error parsing argument pointers");
    
    AST_node* tmp_node = init_tree(AST_TBF, "", symbol_tree, pointer_tree);
    if (tmp_node == NULL) parser_panic("error tree creation");
    return tmp_node;
}

AST_node* parse_type_def() { 
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if((*tokens_p)->id != TYPE) return NULL;
    char* value = (*tokens_p)->str;
    tokens_p++;

    AST_node* arg_node = parse_argument();
    if (arg_node == NULL) parser_panic("error parsing type arguments");
    arg_node->type = AST_TYPE;
    arg_node->value = value;
    return arg_node;
}

AST_node* parse_var_def() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    if ((*tokens_p)->id != VAR) return NULL;
    char* str = (*tokens_p)->str;
    tokens_p++;
    AST_node* var_node = parse_argument();
    if(!var_node) parser_panic("error parsing variable arguments");
    var_node->type = AST_VAR;
    var_node->value = str;
    return var_node;
}
AST_node* parse_place() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    Token** tmp = tokens_p;
    AST_node* node = parse_array_def();
    if (node) return node;
    tokens_p = tmp;

    node = parse_var_def();
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
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    AST_node* tmp = parse_var_def();
    if(tmp) return tmp;
    return parse_type_def();
}
//since top defs are not mandatory, this function always returns null
AST_node* parse_top_defs() {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    AST_node* par = NULL; 
    AST_node* last = NULL;
    AST_node* tmp = NULL;
    do {
        tmp = parse_top_def();
        if(tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");            
            if (!par) par = tmp;
            if (last) last->right_child = tmp;
            last = tmp;
        }
    }while(tmp);

    par = init_tree(AST_TOP_DEF, "", par, NULL);
    if (!par) parser_panic("error tree creation"); 

    return par;
}
AST_node* parse_inc_dec() {
    if(tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    if ((*tokens_p )->id == PLUS || tokens_p[1]->id == PLUS) {
        AST_node* inc = init_tree(AST_INC, "++", NULL, NULL);
        tokens_p += 2;
        return inc;
    }
    if ((*tokens_p )->id == MINUS || tokens_p[1]->id == MINUS) {
        AST_node* dec = init_tree(AST_DEC, "--", NULL, NULL);
        tokens_p += 2;
        return dec;
    }
    return NULL;
}
AST_node* parse_return() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    if ((*tokens_p)->id != RET) return 0;
    tokens_p++;
    Token** tmp = tokens_p;
    AST_node* ret_val = parse_exp();
    if (!ret_val) {
        tokens_p = tmp;
        if ((*tokens_p)->id == SEMI_COL) {
            ret_val = NULL;
            tokens_p++;
        } else parser_panic("invalid return value");
    }

    AST_node* ret = init_tree(AST_RET, "ret", ret_val, NULL);
    if (!ret) parser_panic("error tree creation");
    return ret;
}

AST_node* parse_conditional() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* node = parse_exp();
    if (!node) return NULL;
    return node;
}

AST_node* parse_else() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != ELSE) return NULL;
    tokens_p++;

    AST_node* ifnode = parse_if();
    if (ifnode) {
        ifnode = init_tree(AST_ELSE, "else", ifnode, NULL);
        if(!ifnode) parser_panic("error tree creation");
        return ifnode;
    }
    
    if ((*tokens_p)->id != OPEN_CURL) parser_panic("invalid else");
    tokens_p++;
    
    AST_node* body = parse_statements();
    if (!body) parser_panic("empty else is not permitted");

    if ((*tokens_p)->id != CLOSE_CURL) parser_panic("uneven paranthesis");
    tokens_p++;

    AST_node* else_node = init_tree(AST_ELSE, "else", body, NULL);
    if(!else_node) parser_panic("error tree creation");

    return else_node;
}
AST_node* parse_if() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != IF) return NULL;
    tokens_p++;

    AST_node* cond = parse_conditional();
    if (!cond) parser_panic("expexted a conditional exp");

    if ((*tokens_p)->id != OPEN_CURL) {
        if ((*tokens_p)->id == ASSIGN) parser_panic("assignment inside control statement of if is not permitted. did you mean ==?");
        parser_panic("expected a {");
    }
    tokens_p++;
    
    AST_node* body = parse_statements();
    if (!body) parser_panic("empty if is not permitted");
    
    if ((*tokens_p)->id != CLOSE_CURL) parser_panic("uneven paranthesis");
    tokens_p++;

    AST_node* elsenode = parse_else();

    AST_node* ifnode = init_tree(AST_IF, "if", cond, body);
    if(!ifnode) parser_panic("error tree creation");

    if (elsenode) {
        ifnode = init_tree(AST_IF_ELSE, "", ifnode, elsenode);
        if(!ifnode) parser_panic("error tree creation");
    }

    return ifnode;
}

AST_node* parse_for_init() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    Token** tmp = tokens_p;
    AST_node* node = parse_assignment();
    if(node) return node; 
    tokens_p = tmp;
    
    node = parse_place();
    if (node) return node;
    tokens_p = tmp;
    return NULL;
}

AST_node* parse_for_control() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;
    
    Token** tokens = tokens_p;
    AST_node* cond = parse_conditional();
    if (cond && (*tokens_p)->id == OPEN_CURL) return cond;
    
    tokens_p = tokens;

    AST_node* init = parse_for_init();
    if (!init) parser_panic("probable invalid for syntax");

    if((*tokens_p)->id != SEMI_COL) 
        parser_panic("a semicolon is needed between the three statements of a for statement");
    tokens_p++;

    AST_node* head = init_tree(AST_SEQ, "", init, NULL);
    if (!head) parser_panic("error tree creation");

    cond = parse_conditional();
    if (!cond) parser_panic("a conditional statement is mandatory in for statement");

    AST_node* seq = init_tree(AST_SEQ, "", cond, NULL);
    if (!seq) parser_panic("error tree creation");

    if ((*tokens_p)->id != SEMI_COL) parser_panic("a semicolon is needed between the three statements of a for statement");
    tokens_p++;
    
    if ((*tokens_p)->id != SYMBOL) parser_panic("exptected a symbol");
    AST_node* tmp = init_tree(AST_SYMBOL, (*tokens_p)->str, NULL, NULL); 
    if (!tmp) parser_panic("error tree creation");
    tokens_p++;
    
    AST_node* inc_dec = parse_inc_dec();
    if (!inc_dec) parser_panic("expected increament or decreament");
    inc_dec->left_child = tmp;

    seq->right_child = seq;
    head->right_child = seq;

    return head;
}
AST_node* parse_for() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != FOR) return NULL;
    tokens_p++;
    AST_node* tmp = parse_for_control();
    AST_node* control = init_tree(AST_FOR_CONTROL, "", tmp, NULL);
    if (!control) parser_panic("error tree creation");

    if ((*tokens_p)->id != OPEN_CURL) return NULL;
    tokens_p++;
    AST_node* body = parse_statements();
    if ((*tokens_p)->id != CLOSE_CURL) return NULL;
    tokens_p++;
    AST_node* fornode = init_tree(AST_FOR, "for", control, body);
    return fornode;
}
AST_node* parse_proc_output()  {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_type id;
    char* str;

    if (switch_type((*tokens_p)->id)) id = map_type((*tokens_p)->id);
    else if ((*tokens_p)->id == SYMBOL)  id = AST_SYMBOL;
    else return NULL;

    str = (*tokens_p)->str;
    AST_node* node = init_tree(id, str, NULL, NULL);
    if (!node) parser_panic("error tree creation"); 
    tokens_p++;
    return node;
}

AST_node* parse_proc()  {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    if ((*tokens_p)->id != PROC || tokens_p[1]->id != SYMBOL || 
            tokens_p[2]->id != OPEN_PAR) return NULL;
    char* str = tokens_p[1]->str;
    tokens_p += 3;
    AST_node* tmp = NULL;
    AST_node* head = NULL;
    AST_node* tail = NULL;
    do {
        tmp = parse_argument();
        if (tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!head) head = tmp;
            if (tail) tail->right_child = tmp;
            tail = tmp;
            if ((*tokens_p)->id == COMMA) tokens_p++;
        }
    } while(tmp);
    if ((*tokens_p)->id != CLOSE_PAR) parser_panic("uneven paranthesis");
    tokens_p++;

    AST_node* output = NULL;
    output = parse_proc_output();

    if ((*tokens_p)->id != OPEN_CURL) parser_panic("expected {");
    tokens_p++;
    AST_node* statements = parse_statements();
    if (!statements) parser_panic("expected statements");
    if ((*tokens_p)->id != CLOSE_CURL) parser_panic("uneven curly braces");

    head = init_tree(AST_PROC_INPUT_OUTPUT, "", head, output);
    if (!head)  parser_panic("error tree creation");
    AST_node* proc = init_tree(AST_PROC, str, head, statements);
    if (!proc)  parser_panic("error tree creation");
    return proc;
}  

AST_node* parse_statement() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    Token** tmp = tokens_p;

    AST_node* node = parse_assignment();
    if (node) return node;

    tokens_p = tmp;
    node = parse_array_def();
    if (node) return node;
    
    tokens_p = tmp;
    node = parse_var_def();
    if (node) return node;

    //tokens_p = tmp;
    //node = parse_struct_def();
    //if (node) return node;

    tokens_p = tmp;
    node = parse_if();
    if (node) return node;

    tokens_p = tmp;
    node = parse_for();
    if (node) return node;

    tokens_p = tmp;
    node = parse_func_call();
    if (node) return node;

    tokens_p = tmp;
    node = parse_return();
    if (node) return node;

    return NULL;

}
AST_node* parse_statements() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* tmp = NULL;
    AST_node* head = NULL;
    AST_node* tail = NULL;
    do {
        tmp = parse_statement();
        if(tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL);
            if (!tmp) parser_panic("error tree creation");
            if (!head) head = tmp;
            if (tail) tail->right_child = tmp;
            tail = tmp;
        }
    } while(tmp);
    if (!head) return NULL;

    head = init_tree(AST_STATEMENTS, "", head, NULL);
    if (!head) parser_panic("error tree creation");

    return head;
}
AST_node* parse_combs() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* top_defs = parse_top_defs();
    
    AST_node* tmp = NULL;
    AST_node* head = NULL;
    AST_node* tail = NULL;
    do {
        tmp = parse_proc();
        if(tmp) {
            tmp = init_tree(AST_SEQ, "", tmp, NULL); 
            if (!tmp) parser_panic("error tree creation");
            if (!head) head = tmp;
            if (tail) tail->right_child = tmp;
            tail = tmp;
        }
    }while(tmp);
    if (!head) return NULL;

    AST_node* combs = init_tree(AST_COMBS, "", top_defs, head); 
    if (!combs) parser_panic("error tree creation");
    
    return combs;
}
AST_node* parse_program() {
    if (tokens_p == NULL || (*tokens_p)->id == EOFS) return NULL;

    AST_node* imports = parse_imports();
    
    AST_node* combs = parse_combs();
    if (!combs) return NULL;

    AST_node* program = init_tree(AST_PROGRAM, "", imports, combs);
    if (!program) parser_panic("error tree creation");

    return program;
}
void print_tree(AST_node* node) {
    if (node == NULL) return;
    printf("%d, ", node->type);
    if (node->value != NULL) printf("%s\n", node->value);
    print_tree(node->left_child);
    print_tree(node->right_child);
}
    
AST_node* parse(Stream* input_stream) {
    if (!input_stream) return NULL;
    tokens_p = input_stream->tokens;

    AST_node* program = parse_program();
    if (!program) parser_panic("no program could be parsed");
    print_tree(program);
    return program;
}
