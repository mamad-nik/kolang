#include <stdio.h>
#include <stdlib.h>
#include<stdio.h>
#include<string.h>
#include"../ast/ast.h"
#include"../tokens/tokens.h"
#include"../keywords/keywords.h"
#ifndef PARSERH
#define PARSERH

AST_node* parse(Stream* input_stream); 
#endif
