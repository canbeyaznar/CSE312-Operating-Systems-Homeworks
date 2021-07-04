# Can BEYAZNAR
# 161044038

.data

newLine: .asciiz "\n"

init_name: .asciiz "init"
binarysearch_name: .asciiz "BinarySearch.asm"
linearsearch_name: .asciiz "LinearSearch.asm"
collatz_name: .asciiz "Collatz.asm"
palindrome_name: .asciiz "Palindrome.asm"
interrupt_handle_msg: .asciiz "\nInterrupt handler is working\n"

processID: .word 0 1 2 3 4 5
currentPC: .word 0 0 0 0 0 0
endPC: .word 0 0 0 0 0 0
stackPointerAddress: .word 0 0 0 0 0 0
parentProcess: .word 0 0 0 0 0 0



register0: 
        .align 2
        .space 128

register1: 
        .align 2
        .space 128
register2: 
        .align 2
        .space 128        
register3: 
        .align 2
        .space 128 
		       
register4: 
        .align 2
        .space 128

register5: 
        .align 2
        .space 128


.text
.globl main

main:
    # this part prevents the program from being interrupt
    # main part should not be interrupt until all processes are finished
    li $v0, 20
    syscall

    ######################################################
    ######### Assign the addresses of parameters #########
    ##########In this section, the addresses of########### 
    ##############the arrays in the assembly##############
    ############will be sent to the parameters############
    #############in the syscall.cpp file.#################
    ######################################################

	li $v0, 54
    la $a0, processID
    syscall

    li $v0, 55
    la $a0, endPC
    syscall

    li $v0, 56
    la $a0, stackPointerAddress
    syscall

    li $v0, 57
    la $a0, parentProcess
    syscall

    li $v0, 58
    la $a0, currentPC
    syscall

	li $v0, 59
    la $a0, interrupt_handler
    syscall

    li $v0, 62
    la $a0, exit
    syscall

	li $v0, 63
	la $a0, binarysearch_name
	syscall

	li $v0, 63
	la $a0, linearsearch_name
	syscall

	li $v0, 63
	la $a0, collatz_name
	syscall

	li $v0, 63
	la $a0, palindrome_name
	syscall

	###Buraya palindrome ekle

	li $25, 0
    addi $22, $0,0

	##############################################
    ######### INITIALIZE THE PROCESSES ###########
    ##############################################
    # init process
    la $a0, register0
    li $v0, 19
    la, $t6, init_name
    syscall

    addi $t0, $0,0

    sw  $15, currentPC($t0)
    sw  $24, endPC($t0)
        
        #jal printOneProcess
        addi $t0, $t0, 4    ## increase index

	li $v0, 42
	la $a0, 4
	syscall

	## random process
	la $a0, register1
    li $v0, 21
    syscall


    sw  $15, currentPC($t0)
    sw  $24, endPC($t0)
        
        #jal printOneProcess
        addi $t0, $t0, 4    ## increase index
	
	## random process
	la $a0, register2
    li $v0, 21
    syscall


    sw  $15, currentPC($t0)
    sw  $24, endPC($t0)
        
        #jal printOneProcess
        addi $t0, $t0, 4    ## increase index

	## random process
	la $a0, register3
    li $v0, 21
    syscall

    sw  $15, currentPC($t0)
    sw  $24, endPC($t0)
        
        #jal printOneProcess
        addi $t0, $t0, 4    ## increase index


	## random process
	la $a0, register4
    li $v0, 21
    syscall

    sw  $15, currentPC($t0)
    sw  $24, endPC($t0)
        
        #jal printOneProcess
        addi $t0, $t0, 4    ## increase index


	## random process
	la $a0, register5
    li $v0, 21
    syscall

    sw  $15, currentPC($t0)
    sw  $24, endPC($t0)
        
        #jal printOneProcess
        addi $t0, $t0, 4    ## increase index


	addi $t6, $0, 0
    addi $t0, $0, 0

    li $v0, 20
    syscall


loop:
    j loop
	j interrupt_handler

interrupt_handler:

    # Back up the registers of the previously running process  
    # and upload the new registers to the program
    
    #li $v0, 20
    #syscall


    sw $0, 0($s6)
    #sw $1, process_table+12
    sw $2, 8($s6)
    sw $3, 12($s6)
    sw $4, 16($s6)
    sw $5, 20($s6)
    sw $6, 24($s6)
    sw $7, 28($s6)
    sw $8, 32($s6)
    sw $9, 36($s6)
    sw $10, 40($s6)
    sw $11, 44($s6)
    sw $12, 48($s6)

    sw $13, 52($s6)
    sw $14, 56($s6)
    sw $15, 60($s6)
    sw $16, 64($s6)
    sw $17, 68($s6)
    sw $18, 72($s6)
    sw $19, 76($s6)
    sw $20, 80($s6)
    sw $21, 84($s6)
    #sw $22, 88($s6)

    #sw $23, 92($s6)
    sw $24, 96($s6)
    sw $25, 100($s6)
    #sw $26, 104($s6)
    #sw $27, 108($s6)
    sw $28, 112($s6)
    #sw $29, 116($s6)
    sw $30, 120($s6)
    sw $31, 124($s6)

    ############################
    #new assembly

    lw $0, 0($s7)
    #lw $1, process_table+12
    lw $2, 8($s7)
    lw $3, 12($s7)
    lw $4, 16($s7)
    lw $5, 20($s7)
    lw $6, 24($s7)
    lw $7, 28($s7)
    lw $8, 32($s7)
    lw $9, 36($s7)
    lw $10, 40($s7)
    lw $11, 44($s7)
    lw $12, 48($s7)

    lw $13, 52($s7)
    lw $14, 56($s7)
    lw $15, 60($s7)
    lw $16, 64($s7)
    lw $17, 68($s7)
    lw $18, 72($s7)
    lw $19, 76($s7)
    lw $20, 80($s7)
    lw $21, 84($s7)
    #lw $22, 88($s7)

    #lw $23, 92($s7)
    lw $24, 96($s7)
    lw $25, 100($s7)
    #lw $26, 104($s7)
    #lw $27, 108($s7)
    lw $28, 112($s7)
    #lw $29, 116($s7)
    lw $30, 120($s7)
    lw $31, 124($s7)

    

    # Set Program Counter
    li $v0, 61
    syscall

    #jr $31

exit:
    # if all processes end spimtimer_handler will call
    # this label and kernel will quit
    li $v0,10
    li $v0,10
    syscall