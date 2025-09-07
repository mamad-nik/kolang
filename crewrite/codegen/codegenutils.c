#include"codegen.h"


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

Code_gen* code_gen_init(FILE* output, Table* gv) {
    if (!output || !gv) return NULL;

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
    cg->global_vars = gv;
    data_init(cg);
    text_init(cg);
    bss_init(cg);

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

Symbol* lookup_symbol(Code_gen* cg, char* key) {
    if (!cg || !key) return NULL;
    Symbol* symbol = NULL;
    Entry* entry = NULL;
    if (cg->current_proc) {
         lookup_entry(cg->current_proc->locals, key);
        if (entry) {
           symbol = (Symbol*) entry->value;
           return symbol;
        }
    }
    entry = lookup_entry(cg->global_vars, key);
    if (entry) {
       symbol = (Symbol*) entry->value;
       return symbol;
    }
    return NULL;
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
