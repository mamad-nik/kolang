.section .data
FNV_OFFSET_BASIS: .long 2166136261
FNV_PRIME: .long 16777619 

.section .text
.global _hash

_hash:
	pushq %rbp
	movq %rsp, %rbp
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
	movq %rbp, %rsp
	popq %rbp
	ret
