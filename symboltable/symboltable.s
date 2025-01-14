.set page_size, 4096
.section .text
.global _create_symbol_table
.global _destroy_symbol_table
.global _update_symbol_table
_create_symbol_table:
	movq $9, %rax
	xorq %rdi, %rdi
	movq $page_size, %rsi
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8
	xorq %r9, %r9
	syscall
	movq $page_size, (%rax)
	incq %rax
	ret
_destroy_symbol_table:
	movq %rax, %rdi
	movq $11, %rax 
	movq $page_size, %rsi
	syscall
	ret
_update_symbol_table:
	decq %rax
	movq %rax, %rdi
	movq $19, %rax
	movq (%rdi), %rsi
	movq %rsi, %rdx
	shlq $1, %rdx
	movq $1, %r10
	xorq %r8, %r8
	syscall
	ret

