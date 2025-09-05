#include"codegen.h"

int is_runnable;
Table* global_symbol_table;

int switch_op(AST_type op)  {
    if (op == AST_PLUS || op == AST_MINUS || op == AST_TIMES ||
            op == AST_DIVIDE || op == AST_MODULO || op == AST_AND ||
            op == AST_OR || op == AST_EQ || op == AST_NEQ || op == AST_LT 
            || op == AST_GT || op == AST_LE || op == AST_GE || op == AST_EX_MARK) return 1; 
    return 0;
}

Symbol* lookup_symbol(Table* st, char* key) {
    if (!st || !key) return NULL;
    Symbol* symbol = NULL;
    Entry* entry = lookup_entry(global_symbol_table, key);
    if (entry) {
       symbol = (Symbol*) entry->value;
       return symbol;
    }
    entry = lookup_entry(st, key);
    if (entry) {
       symbol = (Symbol*) entry->value;
       return symbol;
    }
    return NULL;
}
typedef struct { 
    char* buffer;
    size_t size;
    size_t capacity;
} Code_buffer;

Code_buffer* code_buf_init(int size) {
    if (size <= 0) return NULL;
    char* buff = calloc(size , sizeof(char));
    if (!buff) return NULL;
    
    Code_buffer* cbuff =  malloc(sizeof(Code_buffer));
    if (!cbuff) {
        free(buff);
        return NULL;
    }

    cbuff->buffer = buff;
    cbuff->size = 0;
    cbuff->capacity = size;

    return cbuff;
}

Code_buffer* code_buf_resize(Code_buffer* cb, int needed) {
    if (!cb) return NULL;
    if (cb->capacity - cb->size >  needed + 1) return cb;

    size_t new_size = cb->capacity * 2;
    while (new_size < cb->capacity + needed + 1) new_size *= 2;

    char* buff = reallocarray(cb->buffer, new_size, sizeof(char));
    if (!buff) return NULL;

    cb->buffer = buff;
    cb->capacity = new_size;
    return cb;
}

Code_buffer* code_buf_append(Code_buffer* cb, const char* format, ...) { 
    if (!cb) return NULL;
    if (!format) return NULL;

    va_list args;
    va_start(args, format);

    va_list tmp_args;
    va_copy(tmp_args, args);
    int needed = vsnprintf(NULL, 0, format, tmp_args);
    va_end(tmp_args);


    cb = code_buf_resize(cb, needed);
    if (!cb) return NULL;

    vsnprintf(cb->buffer + cb->size, cb->capacity - cb->size, format, args);
    cb->size += needed;
    va_end(args);

    return cb;
} 
Code_buffer* code_buf_concat(Code_buffer* dest, Code_buffer* src) {
    if (!dest || !src) return NULL;

    int size = src->size;
    dest = code_buf_resize(dest, size);
    if (!dest) return NULL;
    
    memcpy(dest->buffer + dest->size, src->buffer, size);
    dest->size += size;

    return dest;
}

void code_buf_destroy(Code_buffer* cb) {
    if (!cb)  return;

    free(cb->buffer);
    free(cb);
}
void code_buf_write_to_file(Code_buffer* cb, FILE* fd) {
    if (!cb || !fd) return;

    if (fwrite(cb->buffer, 1, cb->size, fd) != cb->size) {
       perror("fwrite failed");
       return;
    }
}

typedef struct {
    char* name; 
    int stack_size;
    int current_offset;
    Table* locals;
    Code_buffer* prologue;
    Code_buffer* body;
    Code_buffer* epilogue;
} Proc_cx;

Proc_cx* proc_cx_init(char* name, Table* st) {
    if (!name) return NULL;
    
    Proc_cx* cx = malloc(sizeof(Proc_cx));
    if (!cx) return NULL;

    char* str = strdup(name);
    if (!str) {
        free(cx);
        return NULL;
    }

    cx->prologue = code_buf_init(256);
    if (!cx->prologue) {
        free(str);
        free(cx);
        return NULL;
    }
    cx->epilogue = code_buf_init(256);
    if (!cx->epilogue) {
        free(cx->prologue);
        free(str);
        free(cx);
        return NULL;
    }
    cx->body = code_buf_init(1024);
    if (!cx->body) {
        free(cx->prologue);
        free(cx->epilogue);
        free(str);
        free(cx);
        return NULL;
    }
    cx->name = str;
    cx->stack_size = 0;
    cx->current_offset = -8;
    cx->locals = st;
    return cx;
}

void proc_cx_destroy(Proc_cx* cx) {
    if (!cx) return;
    free(cx->name);
    free(cx->body);
    free(cx->prologue);
    free(cx->epilogue);
    if (cx->locals) free(cx->locals);
    free(cx);
}

typedef struct {
    FILE* output;

    Code_buffer* data;
    Code_buffer* bss;
    Code_buffer* text;
    Code_buffer* global;

    Proc_cx* current_proc;

    Table* global_vars;
    int label_counter;
    Table* temps;
    int temp_counter;

    int current_temp_offset;
} Code_gen;

Code_gen* code_gen_init(FILE* output) {
    if (!output) return NULL;

    Code_gen* cg = malloc(sizeof(Code_gen)); 
    if (!cg) return NULL;
    
    cg->data = code_buf_init(512);
    if (!cg->data) {
        free(cg);
        return NULL;
    }
    
    cg->bss = code_buf_init(512);
    if (!cg->bss) {
        free(cg->bss);
        free(cg);
        return NULL;
    }

    cg->text = code_buf_init(2048);
    if (!cg->text) {
        free(cg->text);
        free(cg->bss);
        free(cg);
        return NULL;
    }

    cg->global = code_buf_init(512);
    if (!cg->global) {
        free(cg->global);
        free(cg->text);
        free(cg->bss);
        free(cg);
        return NULL;
    }
    cg->output = output;

    return cg;
}
void code_gen_destroy(Code_gen* cg) {
    if(!cg) return;

    if (cg->data) code_buf_destroy(cg->data);
    if (cg->text) code_buf_destroy(cg->text);
    if (cg->bss) code_buf_destroy(cg->bss);
    if (cg->global) code_buf_destroy(cg->global);
    if (cg->global_vars) destroy_table(cg->global_vars);

    return;
}
void code_gen_print(Code_gen* cg) {
    if(!cg || !cg->output) return;

    code_buf_write_to_file(cg->data, cg->output);
    code_buf_write_to_file(cg->bss, cg->output);
    code_buf_write_to_file(cg->global, cg->output);
    code_buf_write_to_file(cg->text, cg->output);
} 

void prologue_init(Proc_cx* cx) {
    Code_buffer* cb = cx->prologue;
    cb = code_buf_append(cb, "   pushq %%rbp\n"
            "   movq %%rsp, %%rbp\n");
}
void epilogue_init(Proc_cx* cx) {
    Code_buffer* cb = cx->epilogue;
    cb = code_buf_append(cb, ".L%s_epilogue:\n"
            "   popq %%rbp\n"
            "   ret\n",
            cx->name);
}
void data_init(Code_gen* cg) { 
    if (!cg) return;

    cg->data = code_buf_append(cg->data, ".data\n");
}
void text_init(Code_gen* cg) { 
    if (!cg) return;

    cg->text = code_buf_append(cg->text, ".text\n");
}
void bss_init(Code_gen* cg) {
    if (!cg) return;

    cg->bss =  code_buf_append(cg->bss, ".bss\n");
}

int generate_exp(Code_gen* cg, AST_node* expr) {
    if (!expr || !cg) return 1;

    if (!switch_op(expr->type)) {
        switch (expr->type) {
            case(AST_INTEGER_VAL): 
                cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                        "   movq $%s, %%rax\n", expr->value);
                break;
            case(AST_SYMBOL): {
                Symbol* symbol = 
                    lookup_symbol(cg->current_proc->locals, expr->value); 
                if (symbol) {
                    cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                            "   movq %d(%%rbp), %%rax\n", symbol->stack_offset);
                }
                break;
            }
        }
    }
    switch(expr->type) {
        case(AST_PLUS): 
            generate_exp(cg, expr->left_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                    "   pushq %%rax\n");
            generate_exp(cg, expr->right_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                    "   popq %%rbx\n");
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   addq %%rbx, %%rax\n");
            break;
        case(AST_MINUS):
            generate_exp(cg, expr->left_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rax\n");
            generate_exp(cg, expr->right_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                    "   popq %%rbx\n");
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   subq %%rax, %%rbx\n");
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   movq %%rbx, %%rax\n");
            break;
        case(AST_TIMES):
            generate_exp(cg, expr->left_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rax\n");
            generate_exp(cg, expr->right_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                    "   popq %%rbx\n");
            cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                    "   imulq %%rbx, %%rax\n");
            break;
        default:
            break;
    }
    return 1;
}
void generate_ret(Code_gen* cg, AST_node* tree) {
    if (!tree || !cg) return;
    
    if (tree->type != AST_RET) return;

    AST_node* val = tree->left_child;
    if (val) {
        generate_exp(cg, val);
    } else {
        cg->current_proc->body = code_buf_append(cg->current_proc->body, "       movq $0, %%rax\n");
    }
    cg->current_proc->body = code_buf_append(cg->current_proc->body, "       jmp .L%s_epilogue\n", cg->current_proc->name);
}
void generate_statement(Code_gen* cg, AST_node* tree) {
    if (!tree || !cg) return;

    switch (tree->type) {
        case AST_RET:
            generate_ret(cg, tree);
            break;
        default:
            break;
    }
}
void generate_statements(Code_gen* cg, AST_node* tree) {
    if (!tree || !cg) return;

    if (tree->type != AST_STATEMENTS) return;

    tree = tree->left_child;
    while (tree) {
        if (tree->type == AST_SEQ) {
            generate_statement(cg, tree->left_child);
            tree = tree->right_child;
        } else break;
    }
}

void generate_proc(Code_gen* cg, AST_node* tree) {
    if (!tree || !cg) return;
    
    char *name = tree->value;
    if (!strcmp(name, "main")) {
        is_runnable = 1;
    }
    

    Proc_cx* cx = proc_cx_init(name);
    if (!cx) return;
    cg->current_proc = cx;

    prologue_init(cx);
    epilogue_init(cx);

    generate_statements(cg, tree->right_child);
    
    cg->text = code_buf_append(cg->text, "%s:\n", name);
    cg->text = code_buf_concat(cg->text, cx->prologue);
    cg->text = code_buf_concat(cg->text, cx->body);
    cg->text = code_buf_concat(cg->text, cx->epilogue);

    proc_cx_destroy(cx);
}
void generate_procs(Code_gen* cg, AST_node* tree) {
    if (!tree || !cg) return;

    while (tree) {
        if (tree->type == AST_SEQ) {
            generate_proc(cg, tree->left_child);
            tree = tree->right_child;
        } else break;
    }
}
void generate_combs(Code_gen* cg, AST_node* tree) {
    if (!tree || !cg) return;

    // generate_top_defs(cg, tree->left_child);
    generate_procs(cg, tree->right_child);
}
void generate_program(Code_gen* cg, AST_node* tree) {
    if (!tree || !cg) return;

    // generate_imports(cg, tree->left_child);
    generate_combs(cg, tree->right_child);
}

void add_start(Code_gen* cg) {
    if (!cg) return;

    cg->global = code_buf_append(cg->global, ".global _start\n");
    code_buf_append(cg->text, "_start:\n"
            "    call main\n"
            "    mov %rax, %rdi\n"
            "    mov $60, %rax\n"
            "    syscall\n");
}

FILE* codegen(AST_node* tree, FILE* file, Table* symbol_table) {
    if (!tree || !file || !symbol_table) return NULL;
    Code_gen* cg = code_gen_init(file);
    if (!cg) return NULL;


    generate_program(cg, tree);
    if (is_runnable) add_start(cg);
    code_gen_print(cg);
    code_gen_destroy(cg);
    return file;
}
