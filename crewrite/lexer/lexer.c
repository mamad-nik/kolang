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

int line_no = 1;
Stream *stream;

void skip_white_space(char **ptr) {
    char *la = *ptr;
    while(*la == ' ' || *la == '\t' || *la == '\n') { 
        if (*la == '\n') line_no++;
        la++;
    }
    *ptr = la;
}
int match_del(char ch) {
    if (ch == ' ' || ch == '\t' || ch == EOF || ch == '\n' || ch == '(' || ch == '{' || ch == '[' || ch == ')'
            || ch == '}' || ch == ']' || ch == '\0') return 1;
    return 0;
}

char* lex_symbol(char *sym){ 
    if (!sym) {
        fprintf(stderr, "empty symbol\n");
        return NULL;
    }
    if (((*sym < 'A') || (*sym > 'Z')) && ((*sym < 'a') || ( *sym > 'z'))) {
        fprintf(stderr, "invalid symbol\n", sym);
        return NULL;
    }
    sym++;
    if (match_del(*sym)) return sym;
    while (*sym) {
        if (((*sym < '0') || (*sym > '9')) && ((*sym < 'A') || (*sym > 'Z')) && ((*sym < 'a') || ( *sym > 'z')) && *sym != '_') {
            fprintf(stderr, "invalid symbol\n", sym);
            return NULL;
        }
        sym++;
        if (match_del(*sym)) return sym;
    }
    return NULL;
}
char *lex_integer(char *sym, int* value){
    *value = 0;
    int neg;
    if (!sym) {
        fprintf(stderr, "empty integer\n");
        return NULL;
    }
    if (*sym == '+' || *sym == '-') {
        if (*sym == '-') neg = 1;
        sym++;
    }
    while (*sym) {
        if (*sym > '9' || *sym < '0'){
            printf("%c\n", *sym);
            fprintf(stderr, "invalid integer\n");
            return NULL;
        }
        *value *= 10;
        *value += *sym - '0';
        sym++;
        if (match_del(*sym)){ 
            if (neg) *value *= -1;
            return sym;
        }
    }
    return NULL;
}
//float in kolang is actually an equivalent of a double in c
char *lex_float(char *sym, double* value) {
   *value = 0;
   int neg; 
   if (!sym) {
        fprintf(stderr, "empty float\n");
        return NULL;
   }
   if (*sym == '+' || *sym == '-') {
       if (*sym == '-') neg = 1;
       sym++;
   }
    while (*sym) {
        if (*sym > '9' || *sym < '0'){
            printf("%c\n", *sym);
            fprintf(stderr, "invalid float\n");
            return NULL;
        }
        *value *= 10;
        *value += *sym - '0';
        sym++;
        if (*sym == '.') { 
            break;
        }
    }
    if (*sym == '.') { 
        double temp = 0;
        double i = 1;
        sym++;
        while(*sym) {
            if (*sym > '9' || *sym < '0'){
                printf("%c\n", *sym);
                fprintf(stderr, "invalid float\n");
                return NULL;
            }
            temp += (*sym  - '0') * i;
            i *= i;
            sym++;
            if (match_del(*sym)){ 
                *value += temp;
                if (neg) *value *= -1;
                return sym;
            }
        }
    }
    return NULL;
}
char *lex_string(char *input, int *len) {
       char* ptr = input;
       int len_t = 0;
       if(*ptr != '\"') return NULL;
       ptr++;
       while(*ptr != '\"') {
           ptr++;
           len_t++;
           if (*ptr == '\\') {
               ptr++;
               len_t++;
               if (*ptr == '\"') {
                   ptr++;
                   len_t++;
               }
           }
           if (!*ptr) {
               printf("expected closing \"");
               return NULL;
           }
       }
       ptr++;
       *len = len_t;
       return ptr;
}
char* lex_byte(char *input, char* val) {
    if(*input != '\'') return '\0';
    input++;
    *val = *input;
    input++;
    if(*input != '\'') return '\0';
    input++;
    return input;
}

char* lex_bool(char* str, int* val){
    char* fptr = str;  
    int match;
    char* cases[2] = { "false", "true"};
    for (int i = 0; i < 2; i++) {
        match = 1;
        char* boolval = cases[i];
        while(*boolval) {
            if (*boolval != *str) {
                match = 0;
                break;
            }
            boolval++;
            str++;
        }
        if (match == 1) {
            *val = i;
            return str;
        }
    }
    return NULL;
}
int inst_token(char* fptr, char* sptr, Token* token, keywords id) {
    char* str = strndup(sptr - fptr, fptr);
    if (str == NULL) {
        return 0;
    }
    token->id = id;
    token->loc = fptr;
    token->str = str;
    stream = add_to_stream(stream, token);
    if (stream == NULL) return 0;
    return 1;
}

int pattern_match(char*  str, Token* token, Table* table, Stream* stream) {
    char byteval = 'a';
    char* sptr;
    sptr = lex_byte(str, &byteval);
    if (sptr) {
        if (!inst_token(str, sptr, token, BYTE_VAL)) return NULL;
        return sptr;
    } 
    int intval = 0;
    sptr = lex_integer(str, &intval);
    if (sptr) {
        if (!inst_token(str, sptr, token, INTEGER_VAL)) return NULL;
        return sptr;
    }
    sptr = lex_bool(str, &intval);
    if (sptr) {
        if (!inst_token(str, sptr, token, BOOL_VAL)) return NULL;
        return sptr;
    }
    double floatval = 0;
    sptr = lex_float(str, &floatval);
    if (sptr) {
        if (!inst_token(str, sptr, token, FLOAT_VAL)) return NULL;
        return sptr;
    }
    sptr = lex_string(str, &intval);
    if(sptr) {
        if (!inst_token(str, sptr, token, STR_VAL) return NULL;
        return sptr;
    }
    sptr = lex_symbol(str);
    if(sptr) {
        char* name = strndup(sptr - str, str);
        if (name == NULL) {
            return 0;
        }
        if (look_up_entry(table, name) != NULL) {
            Symbol* symbol = malloc(sizeof(Symbol));
            if (symbol == NULL) return NULL;
            symbol->name = name;
            symbol->loc = str;
            symbol->type = NO_TYPE;
            memset(&symbol->value, 0, sizeof(symbol->value));
            if (!insert_entry(table, symbol->name, symbol)) return NULL;
        }
        if (!inst_token(str, sptr, token, SYMBOL) return NULL;
        return sptr;
    }
    return NULL;
}


int lex(char **input) {
    char *fptr = *input;
    char *sptr = fptr;
    stream = init_stream();
    if (stream == NULL) return 0;
    Table* symbol_table = create_table();
    if (symbol_table == NULL) return 0;
    while(*fptr) {
        int keyword_match = 0;
        skip_white_space(&fptr);
        Token* token = malloc(sizeof(Token));
        for(int i = 0; i < tokens_num; i++) {
            if (token_types[i].name[0] == *fptr) {
                sptr = &fptr[1];
                char* tptr = &token_types[i].name[1];
                int match = 1;
                while(*tptr) {
                    if (*tptr != *sptr) {
                        match = 0; 
                        break;
                    }
                    tptr++; 
                    sptr++;
                }                
                if (match) if (match_del(*sptr)) {
                    inst_token(fptr, sptr, token, i);
                    keyword_match = 1;
                    fptr = sptr;
                    printf("%s, %d\n", token_types[i].name, token_types[i].id);
                    break;
                }
            }
        }
        if (!keyword_match) {
            sptr = fptr;
            sptr = pattern_match(&sptr, token, symbol_table);
            if (sptr == NULL) return 0;
            fptr = sptr;
        }
    }
    return 1;
}
int main() {
    return 0;
}
