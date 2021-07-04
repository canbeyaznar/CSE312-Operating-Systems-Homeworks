    .data
    ListofPrograms: .asciiz "BinarySearch.asm Index: 0\nLinearSearch.asm Index: 1\nCollatz.asm Index: 2\n"
    randomNumbersMessage: .asciiz "The indexes of random number is : "
    newLine: .asciiz "\n"
    space: .asciiz " "
		.text
.globl main

main:
    li $v0, 22
    syscall

    li $v0, 18
    syscall
    jal createRandomNumber

    addi $t0, $a0, 0    #index of first random program 

    la $v0,4
    la $a0,randomNumbersMessage
    syscall
    
    la $v0,1
    addi $a0,$t0,0
    syscall

    la $v0,4
    la $a0,space
    syscall

    jal createRandomNumber
    addi $t1, $a0, 0    #index of second random program 

    beq $t1,$t0,createRandomNumber

    la $v0,1
    addi $a0,$t1,0
    syscall

    la $v0,4
    la $a0,newLine
    syscall

    #FOR FIRST RANDOM PROGRAM
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


    #FOR FIRST RANDOM PROGRAM
    #  1. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t1, 0
    syscall

    #  2. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t1, 0
    syscall

    #  3. program
    li $v0, 19
    syscall

    li $v0, 20
    addi $a0, $t1, 0
    syscall

    li $v0, 22
    syscall

exit:
    beq	$a1, 99, finish	
    j exit

finish:
    li $v0,10
    syscall

createRandomNumber:
    li $v0,42 #random number generator
    la $a0,3
    syscall
    
    jr $ra