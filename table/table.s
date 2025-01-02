.section .data
FNV_OFFSET_BASIS: .quad 14695981039346656037
FNV_PRIME: .quad 1099511628211

.section .text
.global _hash

_hash:
	pushq %rbx
	movq %rax, %rsi
	movq $FNV_OFFSET_BASIS, %rax
	movq $FNV_PRIME, %rcx
loop:
	mulq %rcx
	movq %rax, %rbx

	lodsb 
	test %al, %al
	jz done

	xorq  %rbx, %rax
	jmp loop

done: 
	popq %rbx
	ret
