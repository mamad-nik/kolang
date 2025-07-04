#include"parser.h"

Stream* stream_p;

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
//LOAD
int parse_load(Token** tokens) {
    if(tokens == NULL) return 0;
    if((*tokens)->id != LOAD) return 0;
    tokens++;
    if((*tokens)->id != OPEN_PAR) return 0;
    tokens++;
    if((*tokens)->id != STRING_VAL) return 0;
    tokens++;
    if((*tokens)->id != CLOSE_PAR) return 0;
    tokens++;
    return 1;
}
int parse_imports(Token** tokens) {
    if(tokens == NULL) return 0;
    int c;
    do {
        c = parse_top_def(tokens);
    }while(c)
    return 1;

int parse_type(Token** tokens) { 
    if(tokens == NULL) return 0;
    if((*tokens)->id != TYPE) return 0;
    if(!parse_argument(tokens)) return 0;
    return 1;
}
int parse_argument(Token** tokens) {
    if (tokens == NULL) return 0;
    if ((*tokens)->id != SYMBOL) return 0;
    tokens++;
    if (!switch_type((*tokens)->id)) return 0;
    tokens++;
    return 1;
}
int parse_pointer(Token** tokens) {
    if (tokens == NULL) return 0;
    if ((*tokens)->id != POINTER) return 0;
    tokens++;
    if(!parse_argument((*tokens)->id)) return 0;
    tokens++;
    return 1;
}
int parse_var(Token** tokens) {
    if (tokens == NULL) return 0;
    if ((*tokens)->id != VAR) return 0;
    tokens++;
    if(!parse_argument((*tokens)->id)) return 0;
    tokens++;
    return 1;
}
//TOP_DEF
int parse_gvar(Token** tokens) { 
    if(tokens == NULL) return 0;
    return parse_var(tokens);
}
int parse_top_def(Token** tokens) {
    if(tokens == NULL) return 0;
    int n = parse_gvar(tokens);
    if(n) return n;
    return parse_type(tokens);
}
int parse_top_defs(Token** tokens) {
    if(tokens == NULL) return 0;
    int c;
    do {
        c = parse_top_def(tokens);
    }while(c)
    return 1;
}
int parse_statements(Token** tokens) {
    if (tokens == NULL) return 0;

    if (parse_var(tokens)) return 1;
    if (parse_if(tokens)) return 1;
    if (parse_for(tokens)) return 1;
    if (parse_func_cal(tokens)) return 1;
    return 0;
}
int parse_return(Token** tokens) {
    if (tokens == NULL) return 0;
    if ((*tokens)->id != RET) return 0;
    tokens++;
    //TODO: implement all_val parser
    //if ((*tokens

    return 1;
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
        no_arguments = parse_argument(tokens);
    } while(no_arguments);
    if ((*tokens)->id != CLOSE_PAR) return 0;
    tokens++;



}  

int parse(Stream* input_stream) {
    if (input_stream == NULL) return 0;
    stream_p = input_stream;
    return parse_var(stream_p->tokens);
}
