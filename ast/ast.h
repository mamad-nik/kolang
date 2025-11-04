#ifndef ASTH
#define ASTH
#include"../type_system/type_system.h"
#include"../symbol_table/symbol_table.h"

typedef enum {
    AST_PROC,
    AST_IF,
    AST_ELSE,
    AST_FOR,
    AST_RET,
    AST_VAR,
    AST_LOAD,

    AST_INT,
    AST_BOOL,
    AST_STRING,
    AST_FLOAT,
    AST_BYTE,
    AST_STRUCT,
    AST_POINTER,
    AST_TYPE,
    AST_VALUE,

    AST_EQ,
    AST_NEQ,
    AST_GT,
    AST_LT,
    AST_GE,
    AST_LE,
    
    AST_PLUS,
    AST_MINUS,
    AST_TIMES,
    AST_DIVIDE,
    AST_ASSIGN,
    AST_MODULO,

    AST_SEQ,
    AST_EXPR,
    AST_ERR,
    AST_TBF,
    AST_COND,
    AST_STRUCT_FIELD,
    AST_STRUCT_VALUE,
    AST_ARRAY,
    AST_ARRAY_LIT,
    AST_ARRAY_VALUE,
    AST_ARRAY_RANGE,
    AST_ARRAY_ACCESS,
    AST_LIB,
    AST_INC,
    AST_DEC,
    AST_EX_MARK,
    AST_OR,
    AST_AND,
    AST_FOR_CONTROL,
    AST_ASRGS,
    AST_STATEMENTS,
    AST_PROC_INPUT_OUTPUT,
    AST_IF_ELSE,
    AST_COMBS,
    AST_PROGRAM,
    AST_TOP_DEF,
    AST_WHILE,

    AST_SYMBOL,
    AST_INTEGER_VAL,
    AST_FLOAT_VAL,
    AST_BOOL_VAL,
    AST_STR_VAL,
    AST_BYTE_VAL 
} AST_type;

typedef struct AST_node{
    AST_type type;
    char* value;
    struct AST_node* left_child;
    struct AST_node* right_child;
    Basic_type attr;
    Table* symbols;
} AST_node;

Basic_type map_basic_type(AST_type type);
Basic_type map_basic_val(AST_type id);
AST_node* init_tree(AST_type type, char* value, AST_node* left_child, AST_node* right_child);
#endif
