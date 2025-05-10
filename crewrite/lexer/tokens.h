#ifndef TOKENSH
#define TOKENSH

#include "../keywords/keywords.h"

typedef struct {
    char* name;
    keywords id;
} token;

extern const token tokens[];
extern const int tokens_num;
#endif
