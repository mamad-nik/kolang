#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lexer/lexer.h"
#include "parser/parser.h"

/*
int create_file(char* name) {
    int file = openat(AT_FDCWD, name,  O_RDWR | O_CREAT);     
    if (file < 0) {
        fprintf(stderr, "error creating the file %s, error: %s\n", name, strerror(errno));
        return -1;
    }
    return file; 
}

char* src_name(char* input) {
    size_t size = strlen(input);
    if (size < 2) return NULL;
  
    char *name = malloc(size - 1);
    if (!name) return NULL;

    strncpy(name, input, size - 2);
    name[size - 2] = '\0';
    return name;
}
char* read_all(int file) {
    struct stat st;
    fstat(file, &st);
    off_t size = st.st_size;
    
    char *buffer = malloc(size);
    if (!buffer) return NULL;

    size_t read_size = 0;
    while( read_size < size ) {
           ssize_t n = read(file, buffer + read_size, size - read_size);
           if (n == 0) break;
           if (n < 0) {
                fprintf(stderr, "error while reading the src, error: %s\n", strerror(errno));
                free(buffer);
                return NULL;
           }
           read_size += n;
    }
    return buffer;
        
}
int reader_parser(int file) {
   char* buffer = read_all(file); 
   printf("%s\n", buffer);
   return 0;
}

int open_src(char* name) {
    int file = openat(AT_FDCWD, name, O_RDONLY);
    if (file < 0) {
        fprintf(stderr, "error opening src %s, error: %s\n", name, strerror(errno));
        return -1;
    }
    return file;
}
*/
int print_token(Token* token) {
    if (token != NULL) printf("%d, %u, %s\n", token->id, token->loc, token->str);
}
int print_stream(Stream *stream) {
    if(stream == NULL) return 0;
    printf("---------printing stream---------\n");
    for (size_t i = 0; i < stream->no_tokens; i++) print_token(stream->tokens[i]); 
    printf("---------stream printed---------\n");
}
int print_symbol(Symbol *symbol) {
    if(symbol != NULL) printf("%s, %u\n", symbol->name, symbol->loc);
}
int print_table(Table* table) {
    if (table == NULL) return 0;
    printf("---------printing table---------\n");
    for(int i = 0; i < table->no_buckets; i++) {
        Entry* entry = table->entries[i];
        while(entry != NULL){
            print_symbol((Symbol *)entry);
            entry = entry->next;
        }
    }
    printf("---------table printed---------\n");
}

int main(int argc, char *argv[]) {
    char* string = "var input = 5";
    lex(&string);
    print_stream(stream);
    print_table(symbol_table);
    printf("%d\n", parse(stream));
    
    return 0;
}

