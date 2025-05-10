#include "tokens.h"

const token tokens[] = {
    {"proc", PROC},
    {"if", IF},
    {"for", FOR},
    {"ret", RET},
    {"var", VAR},
    {"load", LOAD},
    {"int", INT},
    {"bool", BOOL},
    {"string", STRING},
    {"float", FLOAT},
    {"byte", BYTE},
    {"==", EQ},
    {">", GT},
    {"<", LT},
    {">=", GE},
    {"<=", LE},
    {"+", PLUS},
    {"-", MINUS},
    {"*", TIMES},
    {"/", DIVIDE},
    {"=", ASSIGN}

};
const int tokens_num = sizeof(tokens) / sizeof(tokens[0]);
