        .data
prompt: .asciiz "Enter 3 integers: "
comma: .asciiz ","
        .text
main_x:        
        li $v0, 4
        la $a0, prompt
        syscall

        #first input
        li $v0, 5
        syscall
        move $s0,$v0
  
        #second input
        li $v0, 5
        syscall
        move $s1,$v0

        #third input
        li $v0, 5
        syscall
        move $s2,$v0

        slt $t3,$s1,$s0 #if firstinput > secondinput, exit
        beq $t3,1,exit

while:
        beq $s0, $s1, exit
                
        div $s3, $s0, $s2
        mul $s4,$s3, $s2
        beq $s4,$s0, printVal
                
        add $s0, $s0, 1

        j while

printVal:
        li $v0, 1
        move $a0, $s0
        syscall

        li $v0, 4
        la $a0, comma
        syscall

        add $s0, $s0, 1
        b while

exit:
        li $v0,10
        syscall