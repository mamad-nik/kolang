#ifndef SEMANTICSH
#define SEMANTICSH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"../ast/ast.h"
#include"../type_system/type_system.h"
#include"../symbol_table/symbol_table.h"
#include "../table/table.h"

void semantics(AST_node* tree, Table* symbol_table); 
#endif
