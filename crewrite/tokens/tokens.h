#ifndef TOKENSH
#define TOKENSH

#include <stdlib.h>
#include <stddef.h>
#include "../keywords/keywords.h"

typedef struct {
    char* name;
    keywords id;
} Token_type;

typedef struct {
    keywords id;
    char* loc;
    char* str;
} Token;

typedef struct {
    Token** tokens;
    size_t no_tokens;
    size_t cap;
} Stream;

extern const Token_type token_types[];
extern const int tokens_num;

void destroy_stream(Stream* stream);
Stream* update_stream(Stream* stream);
Stream* init_stream();
Stream* add_to_stream(Stream* stream, Token* token);
Stream* finilize_stream(Stream* stream, char* fptr);
#endif
