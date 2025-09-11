#include"ircodegen.h"

char* create_formatted_string(const char* format, ...) {
    if (!format) return NULL;
    
    va_list args;
    va_start(args, format);
    
    // Calculate required size
    va_list tmp_args;
    va_copy(tmp_args, args);
    int needed = vsnprintf(NULL, 0, format, tmp_args);
    va_end(tmp_args);
    
    if (needed < 0) {
        va_end(args);
        return NULL;
    }
    
    // Allocate memory
    char* result = malloc(needed + 1);
    if (!result) {
        va_end(args);
        return NULL;
    }
    
    // Format the string
    vsnprintf(result, needed + 1, format, args);
    va_end(args);
    
    return result;
}

CB* cb_init(int size) {
    if (size <= 0) return NULL;
    char* buff = calloc(size , sizeof(char));
    if (!buff) return NULL;
    
    CB* cbuff =  malloc(sizeof(CB));
    if (!cbuff) {
        free(buff);
        return NULL;
    }

    cbuff->buffer = buff;
    cbuff->size = 0;
    cbuff->capacity = size;

    return cbuff;
}

CB* cb_resize(CB* cb, int needed) {
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

CB* cb_append(CB* cb, const char* format, ...) { 
    if (!cb) return NULL;
    if (!format) return NULL;

    va_list args;
    va_start(args, format);

    va_list tmp_args;
    va_copy(tmp_args, args);
    int needed = vsnprintf(NULL, 0, format, tmp_args);
    va_end(tmp_args);


    cb = cb_resize(cb, needed);
    if (!cb) return NULL;

    vsnprintf(cb->buffer + cb->size, cb->capacity - cb->size, format, args);
    cb->size += needed;
    va_end(args);

    return cb;
} 
CB* cb_concat(CB* dest, CB* src) {
    if (!dest || !src) return NULL;

    int size = src->size;
    dest = cb_resize(dest, size);
    if (!dest) return NULL;
    
    memcpy(dest->buffer + dest->size, src->buffer, size);
    dest->size += size;

    return dest;
}

void cb_destroy(CB* cb) {
    if (!cb)  return;

    free(cb->buffer);
    free(cb);
}
void cb_write_to_file(CB* cb, FILE* fd) {
    if (!cb || !fd) return;

    if (fwrite(cb->buffer, 1, cb->size, fd) != cb->size) {
       perror("fwrite failed");
       return;
    }
}

Proc* proc_init(char* name, Table* st) {
    if (!name || !st) return NULL;
    
    Proc* cx = malloc(sizeof(Proc));
    if (!cx) return NULL;

    char* str = strdup(name);
    if (!str) {
        proc_destroy(cx);
        return NULL;
    }

    cx->buffer = cb_init(1024);
    if (!cx->buffer) {
        free(str);
        proc_destroy(cx);
        return NULL;
    cx->locals = st;

    return cx;
}

void proc_destroy(Proc* cx) {
    if (!cx) return;
    if (cx->name) free(cx->name);
    if (cx->buffer) cb_destroy(cx->buffer);
    if (cx->locals) free(cx->locals);
    free(cx);
}

CG* cg_init(FILE* output, Table* gv) {
    if (!output || !gv) return NULL;

    CG* cg = malloc(sizeof(CG)); 
    if (!cg) return NULL;

    cg->perm = cb_init(2048);
    if (!cg->text) {
        cg_destroy(cg);
        return NULL;
    }
    cg->text = cb_init(2048);
    if (!cg->text) {
        cg_destroy(cg);
        return NULL;
    }

    cg->output = output;
    cg->curr_proc = NULL;
    cg->curr_scope = NULL;
    cg->global_table = gv;
    cg->label_counter = 0; 
    cg->temp_counter = 0;
    
    return cg;
}
void cg_destroy(CG* cg) {
    if(!cg) return;

    if (cg->perm) cb_destroy(cg->perm);
    if (cg->text) cb_destroy(cg->text);
    if (cg->curr_proc) proc_destroy(cg->curr_proc);
    free(cg);
}

void cg_print(CG* cg) {
    if(!cg || !cg->output) return;

    cb_write_to_file(cg->perm, cg->output);
    cb_write_to_file(cg->text, cg->output);
} 

Symbol* lookup_symbol(CG* cg, char* key) {
    if (!cg || !key) return NULL;
    Symbol* symbol = NULL;
    Entry* entry = NULL;
    if (cg->curr_scope) {
        entry = lookup_entry(cg->curr_scope, key);
        if (entry) {
           symbol = (Symbol*) entry->value;
           return symbol;
        }
    }

    if (cg->curr_proc) {
        entry = lookup_entry(cg->curr_proc->locals, key);
        if (entry) {
           symbol = (Symbol*) entry->value;
           return symbol;
        }
    }
    entry = lookup_entry(cg->global_table, key);
    if (entry) {
       symbol = (Symbol*) entry->value;
       return symbol;
    }
    return NULL;
}
