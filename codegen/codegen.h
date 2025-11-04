#ifndef codegenh
#define codegenh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../symbol_table/symbol_table.h"
#include "../ast/ast.h"
#include "../type_system/type_system.h"

char* create_formatted_string(const char* format, ...); 
typedef struct { 
    char* buffer;
    size_t size;
    size_t capacity;
} Code_buffer;

Code_buffer* code_buf_init(int size);
Code_buffer* code_buf_resize(Code_buffer* cb, int needed);
Code_buffer* code_buf_append(Code_buffer* cb, const char* format, ...); 
Code_buffer* code_buf_concat(Code_buffer* dest, Code_buffer* src);
void code_buf_destroy(Code_buffer* cb);
void code_buf_write_to_file(Code_buffer* cb, FILE* fd);

typedef struct Epilogue_entry{
    char* instruction;
    struct Epilogue_entry* next;
} Epilogue_entry;

typedef struct {
    Epilogue_entry* top;
    int count;
} Epilogue_stack;

Epilogue_stack* epilogue_stack_init();
int epilogue_stack_push(Epilogue_stack* stack, const char* format, ...);
Code_buffer* epilogue_stack_to_buffer(Epilogue_stack* stack, Code_buffer* buffer);
void epilogue_stack_destroy(Epilogue_stack* stack);

extern const char* caller_saved[];
extern const char* callee_saved[];

typedef struct {
    char* name; 
    int stack_size;
    int current_offset;
    int used_caller[8];
    int used_callee[5];
    Table* locals;
    Code_buffer* prologue;
    Code_buffer* body;
    Code_buffer* epilogue;
    Epilogue_stack* epilogue_stack;
} Proc_cx;

Proc_cx* proc_cx_init(char* name, Table* st);
void proc_cx_destroy(Proc_cx* cx);


typedef struct {
    char* loc;
    int is_in_use;
    int size;
    int align;
} Temp;

typedef struct {
    Temp** temp;
    int temp_counter;
    int capacity;
} Temps;


int get_temp(Temps* temps, int size, int align);
Temp* get_temp_str(Temps* temps, int number);
Temps* create_temps();
void destroy_temps(Temps* temps);

typedef struct {
    FILE* output;

    Code_buffer* data;
    Code_buffer* bss;
    Code_buffer* text;
    Code_buffer* global;

    Proc_cx* current_proc;

    Table* global_vars;
    int label_counter;
    Temps* temps; 

    Table* innest_scope;

    int current_temp_offset;
} Code_gen;

Code_gen* code_gen_init(FILE* output, Table* gv);
void code_gen_destroy(Code_gen* cg);
void code_gen_print(Code_gen* cg);
Symbol* lookup_symbol(Code_gen* cg, char* key);
void prologue_init(Proc_cx* cx);
void epilogue_init(Proc_cx* cx);
void data_init(Code_gen* cg); 
void text_init(Code_gen* cg); 
void bss_init(Code_gen* cg);

FILE* codegen(AST_node* tree, FILE* file, Table* symbol_table);

#endif
