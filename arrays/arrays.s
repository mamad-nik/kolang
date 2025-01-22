.section .data
.set default_size, 10
.set header_size, 32
error_resize_array_str: .asciz "error resizing array, mremap"#29
error_create_array_str: .asciz "error creating array, mmap"#26
.section .text
.extern _resolve_type
.extern _err_exit
.global _create_array
.global _destroy_array
.global _resize_array
.global _add_to_array
#header: type, size, capacity, number of elements
_create_array:
	pushq %rbp
	movq %rsp, %rbp 
	pushq %rbx
	pushq %r12
	pushq %rax 

	call _resolve_type
	movq %rax, %r12
	movq $default_size, %rbx
	mulq %rbx
	movq %rax, %rbx
	
	movq $9, %rax
	xorq %rdi, %rdi
	movq %rbx, %rsi		
	addq $header_size, %rsi
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8 
	xorq %r9, %r9
	syscall

	cmpq $0, %rax
	jl error_create_array
	
	#type
	popq (%rax)
	#size
	movq %r12, 8(%rax)
	#capacity
	movq %rbx, 16(%rax)
	# number of elements set as default: 0
	movq %rax, %rbx

	movq $9, %rax
	xorq %rdi, %rdi
	movq $8, %rsi		
	movq $0x3, %rdx 
	movq $0x22, %r10 
	xorq %r8, %r8 
	decq %r8 
	xorq %r9, %r9
	syscall

	cmpq $0, %rax
	jl error_create_array

	movq %rbx, (%rax)
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret
	error_create_array:
		leaq error_create_array_str(%rip), %rax
		movq $29, %rdi
		call _err_exit
_resize_array:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx

	movq %rax, %rbx

	movq (%rax), %rdi
	movq $0x19, %rax
	movq 16(%rdi), %rsi 
	movq %rsi, %rdx 
	shlq $1, %rdx
	pushq %rdx
	addq $header_size, %rdx
	movq $1, %r10
	xorq %r8, %r8
	syscall

	cmpq $0, %rax
	jl error_resize_array

	popq 16(%rax)	
	movq %rax, (%rbx)
	movq %rbx, %rax

	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret
	error_resize_array:
	leaq error_resize_array_str(%rip),  %rax
	movq $29, %rdi
	call _err_exit
_add_to_array:
	pushq %rbp
	movq %rsp, %rbp	
	pushq %r12
	pushq %rbx
	pushq %rax
	
	movq (%rax), %rbx
	movq 8(%rbx), %r12
	movq 24(%rbx), %rax
	mulq %r12
	cmpq 16(%rbx), %rax
	jl add_to_array_continue
	popq %rax
	pushq %rdi
	call _resize_array
	popq %rdi
	pushq %rax
	movq (%rax), %rbx
	add_to_array_continue:
	movq 24(%rbx), %r12
	movq 8(%rbx), %rax
	cmpq $1, %rax
	je add_to_array_byte
	cmpq $2, %rax
	je add_to_array_word
	cmpq $4, %rax
	je add_to_array_long
	cmpq $8, %rax
	je add_to_array_quad

	add_to_array_byte:
	movb %dil, header_size(%rbx, %r12)
	jmp	add_to_array_done
	add_to_array_word:
	movw %di, header_size(%rbx, %r12)
	jmp	add_to_array_done
	add_to_array_long:
	movl %edi, header_size(%rbx, %r12)
	jmp	add_to_array_done
	add_to_array_quad:
	movq %rdi, header_size(%rbx, %r12)
	add_to_array_done:
	incq 24(%rbx)
	popq %rax
	popq %r12
	movq %rbp, %rsp
	popq %rbp
	ret




