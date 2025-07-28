#include "tokens.h"
#define BASE_SIZE 32

const Token_type token_types[] = {
    {"proc", PROC},
    {"if", IF},
    {"else", ELSE},
    {"for", FOR},
    {"ret", RET},
    {"var", VAR},
    {"load", LOAD},
    {"int", INT},
    {"bool", BOOL},
    {"string", STRING},
    {"float", FLOAT},
    {"byte", BYTE},
    {"struct", STRUCT}, 
    {"pointer", POINTER},
    {"type", TYPE},
    {"value", VALUE},
    {"==", EQ},
    {">", GT},
    {"<", LT},
    {">=", GE},
    {"<=", LE},
    {"+", PLUS},
    {"-", MINUS},
    {"*", TIMES},
    {"/", DIVIDE},
    {"=", ASSIGN},
    {"(", OPEN_PAR},
    {")", CLOSE_PAR},
    {"[", OPEN_BRAC},
    {"]", CLOSE_BRAC},
    {"{", OPEN_CURL},
    {"}", CLOSE_CURL},
    {";", SEMI_COL}, 
    {":", COL},
    {",", COMMA},
    {".", DOT},
    {"!", EX_MARK}
};
const int tokens_num = sizeof(token_types) / sizeof(token_types[0]);

Stream* init_stream() {
    Stream* stream = malloc(sizeof(Stream));
    if (stream == NULL) return NULL;
    stream->tokens = calloc(BASE_SIZE, sizeof(Token *));
    if (stream->tokens == NULL)  {
        free(stream);
        return NULL;
    }
    stream->no_tokens = 0;
    stream->cap = BASE_SIZE;
    return stream;
}
Stream* update_stream(Stream* stream) {
    if(stream == NULL) return NULL;
    if(stream->no_tokens != stream->cap) return stream;
    size_t new_size = BASE_SIZE+stream->cap;
    Token** tmp = reallocarray(stream->tokens, new_size, sizeof(Token*));
    if (tmp == NULL) return NULL;
    stream->tokens = tmp;
    stream->cap = new_size;
    return stream;
}
Stream* add_to_stream(Stream* stream, Token* token) {
    if(token == NULL) return NULL;
    if(stream->no_tokens == stream->cap) stream = update_stream(stream);
    if(stream == NULL) return NULL;
    stream->tokens[stream->no_tokens] = token;
    stream->no_tokens++;
    return stream;
}
Stream* finilize_stream(Stream* stream, char* fptr) {
    if(stream == NULL) return NULL;
    Token* token = malloc(sizeof(Token));
    token->id = EOFS;
    token->loc = fptr;
    token->str = "";
    stream = add_to_stream(stream, token);
    if(stream == NULL) return NULL;
    return stream;
}

void destroy_stream(Stream* stream) {
    if (stream == NULL) return;
    for (size_t i = 0; i < stream->no_tokens; i++) free(stream->tokens[i]);
    free(stream->tokens);
    free(stream);
}
