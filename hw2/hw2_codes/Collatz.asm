.data
    newLine: .asciiz "\n"
    space: .asciiz " "
    doubleDot: .asciiz ": "
    .text

main_h:
    addi $t0, $zero,0
    
loop:
    bgt $t0, 24, exit

    addi $t0,$t0,1
    addi $t2, $t0, 0 # n parameter we will use in printCollatz

    jal printNumber


    li $v0, 22
    syscall
    li $v0, 4
    la $a0,doubleDot
    syscall

    li $v0, 22
    syscall

    jal printCollatz


    li $v0, 22
    syscall
    li $v0, 4
    la $a0,newLine
    syscall
    
    li $v0, 22
    syscall

    j loop

printCollatz:
    beq		$t2, 1, printLastNum


    li $v0, 22
    syscall
    li $v0, 1
    add $a0, $t2, $zero
    syscall

    li $v0, 22
    syscall

    li $v0, 22
    syscall
    li $v0, 4
    la $a0,space
    syscall

    li $v0, 22
    syscall

    #andi $s7, $t2, 1 #if n is odd
    #beq  $s7,1,n_IsOdd  #3*n+1
    li $t5,2
    div		$t2, $t5			# $t0 / $t1
    mflo	$t6					# $t6 = floor($t0 / $t1) 
    mfhi	$t4					# $t4 = $t0 mod $t1 

    beq $t4,1,n_IsOdd
    beq $t4,0,n_IsEven

n_IsOdd:
    mul $t2, $t2,3
    add $t2, $t2,1

    j printCollatz
    
n_IsEven:
    div		$t2,$t2, 2			# $t0 / $t1
    mflo	$t2					# $t2 = floor($t0 / $t1) 


    j printCollatz

printLastNum:
    li $v0, 1
    add $a0, $t2, $zero
    syscall

    li $v0, 22
    syscall

    li $v0, 4
    la $a0,newLine
    syscall

    li $v0, 22
    syscall
    j loop

printNumber:
    li $v0, 1
    add $a0, $t0, $zero
    syscall

    li $v0, 22
    syscall
    li $v0, 4
    la $a0,space
    syscall

    li $v0, 22
    syscall
    
    jr $ra

exit: 
    li $v0, 30
    syscall