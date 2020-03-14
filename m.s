# m.s

		.data
str:	.asciiz "Hello World!\n"

		.text
		.globl main
main:   
		addi $sp,$sp,-4
		sw $ra,0($sp)	# save return address

		li $v0,4	# syscall 4 (print_str)
		la $a0,str 	# load address into $a0
		syscall		# print_str at $a0


		li $v0,12
		syscall	

		move $a0,$v0
		li $v0,1
		syscall	


		li $v0,4	# syscall 4 (print_str)
		la $a0,str 	# load address into $a0
		syscall		# print_str at $a0

		lw $ra,0($sp)
		addi $sp,$sp,4 	# retrieve stack pointer
		jr $ra		# return to the return address

