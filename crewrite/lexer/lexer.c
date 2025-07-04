#include "lexer.h"

Stream *stream;
Table *symbol_table;
int line_no = 1;
char* curr_lexem;

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
            || ch == '}' || ch == ']' || ch == '\0' || ch == ';' || ch == '+' || ch == '-' || ch == '=') return 1;
    return 0;
}

char* lex_symbol(char *beg){ 
    char * sym = beg;
    if (!sym) return NULL;
    if (((*sym < 'A') || (*sym > 'Z')) && ((*sym < 'a') || ( *sym > 'z'))) {
        return NULL;
    }
    sym++;
    if (match_del(*sym)) return sym;
    while (*sym) {
        if (((*sym < '0') || (*sym > '9')) && ((*sym < 'A') || (*sym > 'Z')) && ((*sym < 'a') || ( *sym > 'z')) && *sym != '_') {
            return NULL;
        }
        sym++;
        if (match_del(*sym)) 
            if (curr_lexem != NULL) {
                char* tmp= strndup(beg, sym-beg);
                if (strcmp(curr_lexem, tmp) != 0) return sym;
                return NULL;
            }
    }
    return NULL;
}
char *lex_integer(char *sym, int* value){
    *value = 0;
    int neg;
    if (!sym) return NULL;
    if (*sym == '+' || *sym == '-') {
        if (*sym == '-') neg = 1;
        sym++;
    }
    while (*sym) {
        if (*sym > '9' || *sym < '0') return NULL;
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
   if (!sym) return NULL;
   if (*sym == '+' || *sym == '-') {
       if (*sym == '-') neg = 1;
       sym++;
   }
    while (*sym) {
        if (*sym > '9' || *sym < '0'){
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
               fprintf(stderr, "expected closing \"");
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
    char* str = strndup(fptr, sptr - fptr);
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

char* pattern_match(char*  str, Token* token) {
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
        if (!inst_token(str, sptr, token, STR_VAL))  return NULL;
        return sptr;
    }
    sptr = lex_symbol(str);
    if(sptr) {
        char* name = strndup(str, sptr - str);
        if (name == NULL) {
            return 0;
        }
        if (lookup_entry(symbol_table, name) == NULL) {
            Symbol* symbol = malloc(sizeof(Symbol));
            if (symbol == NULL) return NULL;
            symbol->name = name;
            symbol->loc = str;
            symbol->type = "";
            symbol->is_func = 0;
            symbol->value = NULL;
            if (!insert_entry(symbol_table, symbol->name, symbol)) return NULL;
        }
        if (!inst_token(str, sptr, token, SYMBOL))  return NULL;
        return sptr;
    }
    return NULL;
}

int lex(char **input) {
    char *fptr = *input;
    char *sptr = fptr;
    char *kptr;
    stream = init_stream();
    if (stream == NULL) return 0;
    symbol_table = create_table();
    if (symbol_table == NULL) return 0;
    int keyword_id;
    while(*fptr) {
        keyword_id = -1;
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
                if (match) {
                    kptr = sptr;
                    keyword_id = i;
                    curr_lexem = token_types[i].name;
                    //printf("%s, %d\n", token_types[i].name, token_types[i].id);
                    break;
                }
            }
        }
        Token* patt_token = malloc(sizeof(Token));
        sptr = fptr;
        sptr = pattern_match(sptr, patt_token);
        if (sptr == NULL) {
            if(keyword_id == -1) {
                *input = fptr;
                return 0;
            }
            free(patt_token);
            inst_token(fptr, kptr, token, keyword_id);
            fptr = kptr;
        } else {
            free(token);
            fptr = sptr;
        }
    }
    return 1;
}
