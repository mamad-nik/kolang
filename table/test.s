.section .data
mamad: .asciz "mamad" #6

.section .text
.extern _hash
.global _start

_start: 
	leaq mamad(%rip), %rax
	call _hash
	movq %rax, %rsi
	movq $1, %rax
	movq $1, %rdi
	movq $8, %rdx
	syscall
	movq $60, %rax
	movq $0, %rdi
	syscall
