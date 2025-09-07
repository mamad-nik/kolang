#include"codegen.h"

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

Epilogue_stack* epilogue_stack_init() {
    Epilogue_stack* stack = malloc(sizeof(Epilogue_stack));
    if (!stack) return NULL;
    stack->top = NULL;
    stack->count = 0;
    return stack;
}

int epilogue_stack_push(Epilogue_stack* stack, const char* format, ...) {
    if (!stack || !format) return 0;
    
    va_list args;
    va_start(args, format);
    
    va_list tmp_args;
    va_copy(tmp_args, args);
    int needed = vsnprintf(NULL, 0, format, tmp_args);
    va_end(tmp_args);
    
    char* instruction = malloc(needed + 1);
    if (!instruction) {
        va_end(args);
        return 0;
    }
    
    vsnprintf(instruction, needed + 1, format, args);
    va_end(args);
    
    Epilogue_entry* entry = malloc(sizeof(Epilogue_entry));
    if (!entry) {
        free(instruction);
        return 0;
    }
    
    entry->instruction = instruction;
    entry->next = stack->top;
    stack->top = entry;
    stack->count++;
    
    return 1;
}

Code_buffer* epilogue_stack_to_buffer(Epilogue_stack* stack, Code_buffer* buffer) {
    if (!stack || !buffer) return NULL;
    
    Epilogue_entry* current = stack->top;
    while (current) {
        buffer = code_buf_append(buffer, "%s", current->instruction);
        if (!buffer) return NULL;
        current = current->next;
    }
    
    return buffer;
}

void epilogue_stack_destroy(Epilogue_stack* stack) {
    if (!stack) return;
    
    Epilogue_entry* current = stack->top;
    while (current) {
        Epilogue_entry* next = current->next;
        free(current->instruction);
        free(current);
        current = next;
    }
    free(stack);
}


Proc_cx* proc_cx_init(char* name, Table* st) {
    if (!name) return NULL;
    
    Proc_cx* cx = malloc(sizeof(Proc_cx));
    if (!cx) return NULL;

    char* str = strdup(name);
    if (!str) {
        proc_cx_destroy(cx);
        return NULL;
    }

    cx->prologue = code_buf_init(256);
    if (!cx->prologue) {
        proc_cx_destroy(cx);
        return NULL;
    }
    cx->epilogue = code_buf_init(256);
    if (!cx->epilogue) {
        proc_cx_destroy(cx);
        return NULL;
    }
    cx->body = code_buf_init(1024);
    if (!cx->body) {
        proc_cx_destroy(cx);
        return NULL;
    }
    cx->epilogue_stack = epilogue_stack_init();
    if (!cx->epilogue_stack) {
        proc_cx_destroy(cx);
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
    if (cx->name) free(cx->name);
    if (cx->body) code_buf_destroy(cx->body);
    if (cx->prologue) code_buf_destroy(cx->prologue);
    if (cx->epilogue) code_buf_destroy(cx->epilogue);
    if (cx->epilogue_stack) epilogue_stack_destroy(cx->epilogue_stack);
    if (cx->locals) free(cx->locals);
    free(cx);
}

Code_gen* code_gen_init(FILE* output, Table* gv) {
    if (!output || !gv) return NULL;

    Code_gen* cg = malloc(sizeof(Code_gen)); 
    if (!cg) return NULL;
    
    cg->data = code_buf_init(512);
    if (!cg->data) {
        code_gen_destroy(cg);
        return NULL;
    }
    
    cg->bss = code_buf_init(512);
    if (!cg->bss) {
        code_gen_destroy(cg);
        return NULL;
    }

    cg->text = code_buf_init(2048);
    if (!cg->text) {
        code_gen_destroy(cg);
        return NULL;
    }

    cg->global = code_buf_init(512);
    if (!cg->global) {
        code_gen_destroy(cg);
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
    free(cg);
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
    if (cg->innest_scope) {
        entry = lookup_entry(cg->innest_scope, key);
        if (entry) {
           symbol = (Symbol*) entry->value;
           return symbol;
        }
    }

    if (cg->current_proc) {
        entry = lookup_entry(cg->current_proc->locals, key);
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

Temp* create_temp(int count, int size, int align) {
    Temp* temp = malloc(sizeof(Temp));
    if (!temp) return NULL;
    temp->loc = create_formatted_string("temp%d", count);
    if (!temp->loc) return NULL;
    temp->is_in_use = 0;
    temp->size = size;
    temp->align = align;
    return temp;
}
Temps* create_temps() {
    Temps* temps = malloc(sizeof(Temps));
    if (!temps) return NULL;

    Temp** temp = calloc(16, sizeof(Temp*));
    if (!temp) {
        free(temps);
        return NULL;
    }
    temps->temp = temp;
    temps->capacity = 16;
    temps->temp_counter = 0;
    return temps;
}
Temps* update_temps(Temps* temps) {
    if (!temps) return NULL;
    
    if (temps->capacity > temps->temp_counter) return temps;

    Temp** tmp = reallocarray(temps->temp, temps->capacity*2, sizeof(Temp*));
    if (!tmp) return NULL;

    temps->temp = tmp;
    temps->capacity *= 2;

    return temps;
}
Temps* add_temp(Temps* temps, Temp* temp) {
    if (!temps || !temp) return NULL;

    temps = update_temps(temps);
    if (!temps) return NULL;

    temps->temp[temps->temp_counter] = temp;
    temps->temp_counter++;

    return temps;
}
void print_temps(Code_gen* cg, Temps* temps) {
    if (!temps) return;
    for (int i = 0; i < temps->temp_counter; i++) {
        cg->bss = code_buf_append(cg->bss, ".align %d\n"
                "%s:\n"
                "   .space %d\n",
                temps->temp[i]->align, temps->temp[i]->loc,
                temps->temp[i]->size);
    }

}
int get_temp(Temps* temps, int size, int align) {
    if (!temps) return -1;

    for (int i = 0; i < temps->temp_counter; i++) 
        if (!temps->temp[i]->is_in_use) 
            if (temps->temp[i]->size == size && temps->temp[i]->align == align)
                return i;

    Temp* temp = create_temp(temps->temp_counter, size, align);
    if (!temp) return -1;

    temps = add_temp(temps, temp);
    if (!temps) return -1;

    return temps->temp_counter-1;
}

void prologue_init(Proc_cx* cx) {
    if (!cx) return;
    cx->prologue = code_buf_append(cx->prologue, "   pushq %%rbp\n"
            "   movq %%rsp, %%rbp\n");
}
void epilogue_init(Proc_cx* cx) {
    if (!cx) return;
    epilogue_stack_push(cx->epilogue_stack, "   ret\n");
    epilogue_stack_push(cx->epilogue_stack, "   popq %%rbp\n");
    cx->epilogue = code_buf_append(cx->epilogue, ".L%s_epilogue:\n", cx->name);
    cx->epilogue = epilogue_stack_to_buffer(cx->epilogue_stack, cx->epilogue);
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
