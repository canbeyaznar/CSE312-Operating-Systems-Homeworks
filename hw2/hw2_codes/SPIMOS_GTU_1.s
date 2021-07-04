		.text
        
.globl main

main:

    li $v0, 22
    syscall

    li $v0, 18
    syscall

    li $v0, 19
    syscall

    li $v0, 20
    la $a0, 0
    syscall
    
    li $v0, 19
    syscall

    li $v0, 20
    la $a0, 1
    syscall

    li $v0, 19
    syscall

    li $v0, 20
    la $a0, 2
    syscall


    li $v0, 22
    syscall

exit:
    beq	$a1, 99, finish	
    j exit

finish:
    li $v0,10
    syscall