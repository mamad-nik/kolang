#include<stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

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
int main(int argc, char *argv[]) {
    char *symbols[] = {"mamad", "Mamad", "_return", "8amad", "asdf8"};
    size_t n = sizeof(symbols)/sizeof(symbols[0]);
    for (int i = 0; i < n; i++) {
        parse_symbol(symbols[i]); 
    }
    return 0;
}

