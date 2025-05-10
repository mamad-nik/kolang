#ifndef KEYWORDS_H
#define KEYWORDS_H
typedef enum  {
    PROC,
    IF,
    FOR,
    RET,
    VAR,
    LOAD,

    INT,
    BOOL,
    STRING,
    FLOAT,
    BYTE,

    EQ,
    GT,
    LT,
    GE,
    LE,
    
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    ASSIGN,

    SYMBOL,
    INTEGER_VAL,
    FLOAT_VAL,
    BOOL_VAL,
    STR_VAL,
    BYTE_VAL 
} keywords;
#endif
