.section .text
.global _exit
_exit:
	movq $60, %rax
	movq $0, %rdi
	syscall

