#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantics/semantics.h"

FILE* create_file(char* name) {
    FILE* file = fopen(name, "w+");
    if (!file) {
        fprintf(stderr, "error creating the file %s, error: %s\n", name, strerror(errno));
        return NULL;
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
char* target_name(char* fn) {
    if (!fn) return NULL;

    char* fn1 = strdup(fn);

    char* str = strtok(fn1, ".");
    char* out = strdup(str);
    out = strcat(out, ".s");
    free(fn1);
    return out;
}
char* read_all(int file) {
    struct stat st;
    if (fstat(file, &st) < 0) {
        fprintf(stderr, "fstat failed: %s\n", strerror(errno));
        return NULL;
    }

    off_t size = st.st_size;
    if (size <= 0) {
        fprintf(stderr, "file is empty or size invalid\n");
        return NULL;
    }

    char *buffer = malloc(size + 1); // +1 for '\0' if text
    if (!buffer) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }

    size_t read_size = 0;
    while (read_size < (size_t)size) {
        ssize_t n = read(file, buffer + read_size, size - read_size);
        if (n == 0) break; // EOF
        if (n < 0) {
            fprintf(stderr, "error reading file: %s\n", strerror(errno));
            free(buffer);
            return NULL;
        }
        read_size += n;
    }

    buffer[read_size] = '\0'; 

    //get rid of the trailing white space in the src file
    int wscounter = 0;
    for (int i = read_size -1; i > 0; i--)
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t') wscounter++;
        else break;
    buffer[read_size - wscounter] = '\0';

    return buffer;
}

int open_src(char* name) {
    int file = openat(AT_FDCWD, name, O_RDONLY);
    if (file < 0) {
        fprintf(stderr, "error opening src %s, error: %s\n", name, strerror(errno));
        return -1;
    }
    return file;
}
void print_token(Token* token) {
    if (token != NULL) printf("%d, %s\n", token->id, token->loc);
}
int print_stream(Stream *stream) {
    if(stream == NULL) return 0;
    printf("---------printing stream---------\n");
    for (size_t i = 0; i < stream->no_tokens; i++) print_token(stream->tokens[i]); 
    printf("---------stream printed---------\n");
    return 1;
}
void print_symbol(Symbol *symbol) {
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
    if (argc < 2) return 1;
    int fd = open_src(argv[1]);
    if (fd == -1) return 1;
    char* string = read_all(fd);
    lex(&string);
    print_stream(stream);
    print_table(symbol_table);
    AST_node* tree = parse(stream);
    if (!tree) return -1;
    init_type_registry();
    semantics(tree);
    /*char* str = target_name(argv[1]);
    FILE* file = create_file(str);
    fclose(file);*/
    //codegen(tree, stdout, global_symbol_table);
    
    return 0;
}

