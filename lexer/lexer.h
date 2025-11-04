#ifndef LEXERH
#define LEXERH
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../tokens/tokens.h"
#include "../table/table.h"
#include "../symbol_table/symbol_table.h"


extern Stream *stream;
extern Table *symbol_table;

extern void lex(char **input); 
#endif
