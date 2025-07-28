#include<stdio.h>
#include"../tokens/tokens.h"
#include"../keywords/keywords.h"

/*typedef enum {
    LEFT, 
    RIGHT
} Associativity;
*/
typedef struct {
    char op; 
    int prec;
//    Associativity assoc; 
} Opentry; 

static Opentry optable[] = {
    {'+', 10}, 
    {'-', 10}, 
    {'*', 20}, 
    {'/', 20}, 
    {'%', 20} 
};
typedef struct {
    int is_op;
    char* value;
    

Token** parse_arith_primary(Token** tokens) {
    if (tokens == NULL) return NULL;

    if ((*tokens)->id == OPEN_PAR) {
        tokens = arith_expr(tokens);
        if ((*tokens)->id == CLOSE_PAR) return NULL;
        tokens++;
        return tokens; 
    }
    
    //TODO: add array_access, struct_access and func_cal

    if ((*token)->id == INT_VAL || 
            (*token)->id == FLOAT_VAL ||  (*token)->id == SYMBOL) return tokens
    
}

Token** arith_expr_1(Token** tokens, lhs) {}

Token** arith_expr(Token** tokens) {
    if (tokens == NULL) return NULL;

    parse_arith_primary(tokens); 
