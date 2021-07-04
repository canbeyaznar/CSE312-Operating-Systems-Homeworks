    .data
    ListofPrograms: .asciiz "BinarySearch.asm Index: 0\nLinearSearch.asm Index: 1\nCollatz.asm Index: 2\n"
    newLine: .asciiz "\n"
    randomNumberMessage: .asciiz "The index of random number is : "
		.text
.globl main

main:
    li $v0, 22
    syscall


    li $v0, 18
    syscall

    li $v0, 4
    la $a0, ListofPrograms
    syscall

    li $v0,42 #random number generator
    la $a0,3
    syscall

    addi $t0, $a0,0

    li $v0, 4
    la $a0, randomNumberMessage
    syscall

    li $v0, 1
    addi $a0, $t0, 0
    syscall

    li $v0, 4
    la $a0, newLine
    syscall
    

    #  1. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  2. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  3. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  4. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  5. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  6. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall


    #  7. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  8. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  9. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    #  10. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t0, 0
    syscall

    li $v0, 22
    syscall

exit:
    beq	$a1, 99, finish	
    j exit

finish:
    li $v0,10
    syscall