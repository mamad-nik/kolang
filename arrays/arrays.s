.section .data
.set array_header_size, 32
error_resize_array_str: .asciz "error resizing array, mremap"#29
error_create_array_str: .asciz "error creating array, mmap"#26
.section .text
.global _create_array
.global _destroy_array
.global _resize_array
.global _append_array
.global _add_to_array
.global _destroy_array
.global _get_array_value
#header: data pointer, elem_size, capacity, number of elements 
_create_array:
	pushq %rbp
	movq %rsp, %rbp 
	pushq %rbx
	pushq %r12
	pushq %r13

	movq %rdi, %r12
	movq %rsi, %r13

	cmpq $8, %r12
	jg .error_too_large

	movq %rsi, %rax
	mulq %rdi
	movq %rax, %rsi

	movq $9, %rax
	xorq %rdi, %rdi
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8 
	xorq %r9, %r9
	syscall

	cmpq $-1, %rax
	je .error_create_array

	pushq %rax

	movq $9, %rax
	xorq %rdi, %rdi
	movq $array_header_size, %rsi
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8 
	xorq %r9, %r9
	syscall

	cmpq $-1, %rax
	je .error_create_array

	popq %rbx
	movq %rbx, 0(%rax)
	movq %r12, 8(%rax)
	movq %r13, 16(%rax)
	movq $0, 24(%rax)
	jmp .create_array_done

	.error_too_large:
	movq $0, %rax
	jmp .create_array_done
	
	.error_create_array:
	movq $-1, %rax

	.create_array_done:
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
	ret
_resize_array:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14

	movq %rdi, %rbx 

	movq 16(%rbx), %rax
	testq %rax, %rax
	jnz .normal_resize
	movq $1, %rax

	.normal_resize:
	movq $2, %rcx
	mulq %rcx
	movq %rax, %r12

	movq 8(%rbx), %rcx
	mulq %rcx
	movq %rax, %r13

	movq $9, %rax
	xorq %rdi, %rdi
	movq %r13, %rsi
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8 
	xorq %r9, %r9
	syscall

	cmpq $-1, %rax
	je .error_resize_array

	movq %rax, %r14

	movq 24(%rbx), %rax
	mulq 8(%rbx)
	movq %rax, %rcx

	testq %rcx, %rcx
	jz .resize_skip_copy

	movq 0(%rbx), %rsi
	movq %r14, %rdi	
	rep movsb
	
	.resize_skip_copy:
	movq 16(%rbx), %rax
	mulq 8(%rbx)
	movq %rax, %r13

	movq $11, %rax
	movq 0(%rbx), %rdi
	movq %r13, %rsi
	syscall

	movq %r14, 0(%rbx)
	movq %r12, 16(%rbx)
	xorq %rax, %rax
	jmp .resize_done

	.error_resize_array:
	movq $-1, %rax

	.resize_done:
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
	ret

_append_array:
	pushq %rbp
	movq %rsp, %rbp	
	pushq %r12
	pushq %rbx
	pushq %r13
	
	movq %rdi, %rbx

	movq 24(%rbx), %rax
	incq %rax
	cmpq 16(%rbx), %rax
	jge .append_array_resize


	.append_array_continue:
	movq 0(%rbx), %r12
	movq 24(%rbx), %rax
	mulq 8(%rbx)
	addq %r12, %rax
	movq %rax, %r13

	movq 8(%rbx), %rax
	cmpq $1, %rax
	je .append_array_byte
	cmpq $8, %rax
	je .append_array_quad
	jmp .append_array_error

	.append_array_byte:
	movb %sil, (%r13)
	jmp	.append_array_done
	.append_array_quad:
	movq %rsi, (%r13)
	.append_array_done:
	incq 24(%rbx)
	xorq %rax, %rax
	jmp .append_array_exit

	.append_array_resize:
	movq %rsi, %r13
	movq %rbx, %rdi
	call _resize_array
	testq %rax, %rax
	jnz .append_array_error
	movq %r13, %rsi
	jmp .append_array_continue

	.append_array_error:
	movq $-1, %rax

	.append_array_exit:
	popq %r13
	popq %rbx
	popq %r12
	popq %rbp
	ret

_add_to_array:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	#rdi: pointer,
	#rsi: element,
	#rdx: index,

	movq %rdi, %rbx

	movq 24(%rbx), %rax
	cmpq %rdx, %rax
	jle .add_to_array_error

	movq 8(%rbx), %rax
	mulq %rdx
	addq (%rbx), %rax
	movq %rax, %r12
	
	movq 8(%rbx), %rax
	cmpq $1, %rax
	je .add_to_array_byte
	cmpq $8, %rax
	je .add_to_array_quad
	jmp .add_to_array_error

	.add_to_array_byte:
	movb %sil, (%r12)
	jmp .add_to_array_done
	.add_to_array_quad:
	movq %rsi, (%r12)
	
	.add_to_array_done:
	movq $0, %rax
	jmp .add_to_array_exit

	.add_to_array_error:
	movq $-1, %rax

	.add_to_array_exit:
	popq %r12
	popq %rbx
	popq %rbp
	ret
#rdi: pointer, rsi: index
_get_array_value:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	
	movq %rdi, %rbx
	movq %rsi, %r12

	movq 24(%rbx), %rax
	cmpq %rsi, %rax
	jle .get_array_value_error

	movq 8(%rbx), %rax
	mulq %r12
	addq (%rbx), %rax
	movq %rax, %r13

	movq 8(%rbx), %rax
	cmpq $1, %rax
	je .get_array_byte
	cmpq $8, %rax
	je .get_array_quad
	jmp .get_array_value_error

	.get_array_byte:
	xorq %rax, %rax
	movb (%r13), %al
	jmp .get_array_value_exit

	.get_array_quad:
	movq (%r13), %rax
	jmp .get_array_value_exit
	
	.get_array_value_error:
	movq $-1, %rax
	.get_array_value_exit:
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
	ret

_destroy_array:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx

	movq %rdi, %rbx

	movq 8(%rbx), %rax
	mulq 16(%rbx)
	movq %rax, %rsi
	movq $11, %rax
	movq (%rbx), %rdi
	syscall

	movq $11, %rax
	movq %rbx, %rdi
	movq $array_header_size, %rsi
	syscall

	xorq %rax, %rax

	popq %rbx
	popq %rbp
	ret
