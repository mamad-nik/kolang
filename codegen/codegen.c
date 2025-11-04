#include"codegen.h"

int is_runnable;

int cg_switch_op(AST_type op)  {
    if (op == AST_PLUS || op == AST_MINUS || op == AST_TIMES ||
            op == AST_DIVIDE || op == AST_MODULO || op == AST_AND ||
            op == AST_OR || op == AST_EQ || op == AST_NEQ || op == AST_LT 
            || op == AST_GT || op == AST_LE || op == AST_GE || op == AST_EX_MARK) return 1; 
    return 0;
}
int gen_get_size(Type_info* type) {
    if (!type) return -1;

    switch (type->category) {
        case (TC_STRUCT):
            return 8;
            break;
        case(TC_ARRAY):
            return 8;
            break;
        case(TC_POINTER):
            return 8;
            break;
        case(TC_BASIC):
            return get_basic_type_size(type->data.basic);
            break;
        default:
            return -1;
    }
    return -1;
}
char* store_basic_stack(Type_info* type) {
    if(!type) return NULL;

    if(type->category != TC_BASIC) return NULL;

    switch(type->data.basic) {
        case(BASIC_INT):
            return "movq ";
            break;
        case(BASIC_FLOAT):
            return "movq ";
            break;
        case(BASIC_BYTE):
            return "movzxbq ";
            break;
        case(BASIC_BOOL):
            return "movzxbq ";
            break;
        case(BASIC_STRING):
            return "movq ";
            break;
        default:
            return NULL;
    }
    return NULL;
}
char* store_basic_temp(Type_info* type) {
    if(!type) return NULL;

    if(type->category != TC_BASIC) return NULL;

    switch(type->data.basic) {
        case(BASIC_INT):
            return "movq ";
            break;
        case(BASIC_FLOAT):
            return "movq ";
            break;
        case(BASIC_BYTE):
            return "movb ";
            break;
        case(BASIC_BOOL):
            return "movb ";
            break;
        case(BASIC_STRING):
            return "movq ";
            break;
        default:
            return NULL;
    }
    return NULL;
}

int generate_pointer(Code_gen* cg, char* name) {
    if (!cg || !name) return 0;
}

int generate_array(Code_gen* cg, char* name) {
    if (!cg || !name) return 0;

    Type_info* type = get_type_str(name);
    if (!type) return 0;
    if (type->category != TC_ARRAY) return 0;

    int size = gen_get_size(type->data.array.element_type);
    int no_elem = type->data.array.no_elements;

    cg->current_proc->body = code_buf_append(cg->current_proc->body,
            "pushq %%rbx\n");

    cg->current_proc->body = code_buf_append(cg->current_proc->body,
            "pushq %%rdi\n"
            "pushq %%rsi\n"
            "movq $%d, %%rdi\n"
            "movq $%d, %%rsi\n"
            "call _create_array"
            "cmpq $-1, %%rax\n"
            "je _error_memory_allocation\n"
            "popq %%rsi\n"
            "popq %%rdi\n", size, no_elem);
}
    
int generate_struct(Code_gen* cg, char* name) {
    if (!cg || !name) return 0;

    Type_info* type = get_type_str(name);
    if (!type) return 0;
    if (type->category != TC_STRUCT) return 0;

    int size = type->size_byte;
    int no_fields = type->data.structure.no_fields;


    cg->current_proc->body = code_buf_append(cg->current_proc->body,
            "pushq %%rbx\n");
        

    cg->current_proc->body = code_buf_append(cg->current_proc->body,
            "pushq %%rdi\n"
            "movq $%d, %%rdi\n"
            "call _create_struct\n"
            "cmpq $-1, %%rax\n"
            "je _error_memory_allocation\n"
            "popq %%rdi\n", size);

    for (int i = 0; i < no_fields; i++) {
        int offset = type->data.structure.fields[i]->offset;
        switch (type->data.structure.fields[i]->type->category) {
            case(TC_BASIC):
                switch (type->data.structure.fields[i]->type->data.basic) {
                    case (BASIC_INT):
                        cg->current_proc->body = code_buf_append(cg->current_proc->body, "movq $0, %d(%%rax)\n", offset); 
                        break;
                    case (BASIC_BOOL):
                        cg->current_proc->body = code_buf_append(cg->current_proc->body, "movb $0, %d(%%rax)\n", offset); 
                        break;
                    case (BASIC_BYTE):
                        cg->current_proc->body = code_buf_append(cg->current_proc->body, "movb $0, %d(%%rax)\n", offset); 
                        break;
                    case (BASIC_FLOAT):
                        cg->current_proc->body = code_buf_append(cg->current_proc->body, "movq $0, %d(%%rax)\n", offset); 
                        break;
                    case (BASIC_STRING):
                        cg->current_proc->body = code_buf_append(cg->current_proc->body, "movq $0, %d(%%rax)\n", offset); 
                        break;
                    default:
                        break;
                }
                break;
            case(TC_POINTER):
                //generate_pointer();
                break;
            case(TC_STRUCT):
                cg->current_proc->body = code_buf_append(cg->current_proc->body,
                        "pushq %%rax\n"); 
                generate_struct(cg, type->data.structure.fields[i]->type->name);
                cg->current_proc->body = code_buf_append(cg->current_proc->body,
                        "movq %%rax, %%rbx\n"
                        "popq %%rax\n"
                        "movq %%rbx, %d(%%rax)\n", offset); 
                break;
            default:
                break;
        }
    }
    
    cg->current_proc->body = code_buf_append(cg->current_proc->body,
            "popq %%rbx\n");

    return 1;
}
int generate_struct_access_value(Code_gen* cg, AST_node* tree) {
    if (!cg || !tree || !tree->left_child) return 0;

    char* name = tree->value;
    Symbol* symbol = lookup_symbol(cg, name);
    //movq -16(rip), rbx
    //movq $offset(rbx), %rax
    char* loc = symbol->loc;
    cg->current_proc->body = code_buf_append(cg->current_proc->body,
            "movq %s, %%rbx\n", loc);
    
    tree = tree->left_child;

    AST_node* next = NULL;
    if (tree->type == AST_SEQ) {
        next = tree->right_child;
        if(!tree->left_child) return 0;
        tree = tree->left_child;
    }
    Type_info* type = symbol->type;
    do {
        if (tree->type == AST_SEQ) {
            next = tree->right_child;
            if(!tree->left_child) return 0;
            tree = tree->left_child;
        }
        for (int i = 0; i < type->data.structure.no_fields; i++) {
            if (strcmp(type->data.structure.fields[i]->name,
                        tree->value) == 0) {
                int offset = type->data.structure.fields[i]->offset;
                int size = type->data.structure.fields[i]->type->size_byte;
                type = type->data.structure.fields[i]->type;
                if (size == 8) {
                    cg->current_proc->body =
                        code_buf_append(cg->current_proc->body,
                                "movq %d(%%rbx), %%rax\n", offset); 
                } else if (size == 1) {
                    cg->current_proc->body =
                        code_buf_append(cg->current_proc->body,
                                "xorq %%rax, %%rax\n"
                                "movb %d(%%rbx), %%al\n", offset); 
                }
                
                break;
            }
        }
        if (next) {
            cg->current_proc->body =
                code_buf_append(cg->current_proc->body,
                        "movq %%rax, %%rbx\n"); 
            tree = tree->right_child;
        }
    } while(next);
    return 1;
    
}
int generate_destroy_struct(Code_gen* cg, AST_node* tree) {
    if (!cg || !tree) return 0;

}
int generate_vars(Code_gen* cg, AST_node* tree) {
    if (!cg || !tree) return -1;

    Table* currt = cg->current_proc->locals;
    if (!currt) return -1;

    
    for (int i = 0; i < currt->no_buckets; i++) {
        Entry* entry = currt->entries[i];
        while(entry) {
            Symbol* symbol = (Symbol*) symbol;


            entry = entry->next;
        }

}

int generate_gvars(Code_gen* cg) {
    if (!cg) return 0;
    if (!cg->global_vars) return 0;

    for (int i = 0; i < cg->global_vars->no_buckets; i++) {
        Entry* entry = cg->global_vars->entries[i];
        while(entry) {
            Symbol* symbol = (Symbol *) entry->value;
            if (!symbol) return 0;
            int type_size = 0;
            int align = 0;
            if (symbol->scope == SCOPE_GLOBAL) {
                switch(symbol->type->category) {
                    case (TC_BASIC):
                        type_size = get_basic_type_size(symbol->type->data.basic);
                        align = get_basic_type_align(symbol->type->data.basic);
                        break;
                    case (TC_POINTER):
                        type_size = 8;
                        align = 8;
                        break;
                    case (TC_ARRAY):
                        type_size = 8;
                        align = symbol->type->alignment;
                        break;
                    case (TC_STRUCT):
                        type_size = 8;
                        align = symbol->type->alignment;
                        break;
                    case (TC_FUNCTION):
                        type_size = -1;
                        break;
                    default:
                        type_size = -1;
                        break;
                }
                if (type_size != -1) {
                    code_buf_append(cg->bss, ".align %d\n"
                            "%s:\n"
                            "   .space %d\n\n",
                            align, symbol->name, type_size);
                }
            }
            entry = entry->next;
        }
    }
    return 1;
}


int generate_exp(Code_gen* cg, AST_node* expr) {
    if (!expr || !cg) return 1;

    if (!cg_switch_op(expr->type)) {
        switch (expr->type) {
            case(AST_INTEGER_VAL): 
                cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                        "   movq $%s, %%rax\n", expr->value);
                break;
            case(AST_SYMBOL): {
                Symbol* symbol = 
                    lookup_symbol(cg, expr->value); 
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
                    "   popq %%rbx\n"
                    "   addq %%rbx, %%rax\n");
            break;
        case(AST_MINUS):
            generate_exp(cg, expr->left_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rax\n");
            generate_exp(cg, expr->right_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                    "   popq %%rbx\n"
                    "   subq %%rax, %%rbx\n"
                    "   movq %%rbx, %%rax\n");
            break;
        case(AST_TIMES):
            generate_exp(cg, expr->left_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rax\n");
            generate_exp(cg, expr->right_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body, 
                    "   popq %%rbx\n"
                    "   imulq %%rbx, %%rax\n");
            break;
        case(AST_DIVIDE):
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rdx\n");
            generate_exp(cg, expr->left_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rax\n");
            generate_exp(cg, expr->right_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   movq %%rax, %%rbx\n"
                    "   popq %%rax\n"
                    "   cqo\n"
                    "   idivq %%rbx\n"
                    "   popq %%rdx\n");
            break;
        case(AST_MODULO):
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rdx\n");
            generate_exp(cg, expr->left_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   pushq %%rax\n");
            generate_exp(cg, expr->right_child);
            cg->current_proc->body = code_buf_append(cg->current_proc->body,
                    "   movq %%rax, %%rbx\n"
                    "   popq %%rax\n"
                    "   cqo\n"
                    "   idivq %%rbx\n"
                    "   movq %%rdx, %%rax\n"
                    "   popq %%rdx\n");
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
    Symbol* symbol = lookup_symbol(cg, name);
    
    Proc_cx* cx = proc_cx_init(name, symbol->symbol_table);
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
            "    mov %%rax, %%rdi\n"
            "    mov $60, %%rax\n"
            "    syscall\n");
}

FILE* codegen(AST_node* tree, FILE* file, Table* symbol_table) {
    if (!tree || !file || !symbol_table) return NULL;
    Code_gen* cg = code_gen_init(file, symbol_table);
    if (!cg) return NULL;


    //generate_program(cg, tree);
    //if (is_runnable) add_start(cg);
    generate_gvars(cg);
    code_gen_print(cg);
    code_gen_destroy(cg);
    return file;
}
