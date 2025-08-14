#ifndef KEYWORDS_H
#define KEYWORDS_H
typedef enum  {
    PROC,
    IF,
    ELSE,
    FOR,
    RET,
    VAR,
    LOAD,

    INT,
    BOOL,
    STRING,
    FLOAT,
    BYTE,
    STRUCT,
    POINTER,
    TYPE,
    VALUE,

    EQ,
    NEQ,
    GT,
    LT,
    GE,
    LE,
    
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    MODULO,
    ASSIGN,
    OPEN_PAR,
    CLOSE_PAR,
    OPEN_BRAC,
    CLOSE_BRAC,
    OPEN_CURL,
    CLOSE_CURL,
    SEMI_COL,
    COL,
    COMMA,
    DOT,
    EX_MARK,
    OR,
    AND,

    EOFS,

    SYMBOL,
    INTEGER_VAL,
    FLOAT_VAL,
    BOOL_VAL,
    STR_VAL,
    BYTE_VAL 
} keywords;
#endif
