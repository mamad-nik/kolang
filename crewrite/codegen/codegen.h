#ifndef codegenh
#define codegenh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../symbol_table/symbol_table.h"
#include "../type_system/type_system.h"

FILE* codegen(AST_node* tree, FILE* file);

#endif
