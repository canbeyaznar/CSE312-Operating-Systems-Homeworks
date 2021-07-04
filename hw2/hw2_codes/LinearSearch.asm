#Can BEYAZNAR 161044038

		.data
myArray:	.word 7 14 28 32 44
searchedVal:	.word 28
notFound:	.asciiz		" is not in the list."
Found: .asciiz     " is in the list. \nThe index of value is : "
newLine: .asciiz "\n"
	.text

main_h:
    
        la $s1, myArray #adress of myArray

        li $t1, 0       #index
        li $t2, 1	#control
        li $t3, 5       #size of array

        lw $t0, searchedVal

        j while

while:
        beq $t1, $t3, _notfound     # if the index is out of size the value that we search, could not found
        lw $t4, 0($s1)              #get element in array
        beq $t0, $t4, _found        #if it is equal go to _found

next:
        addi $s1, $s1, 4
	addi $t1, $t1, 1

        j while

_found:
        li $v0, 1
        lw $a0, searchedVal
        syscall

        li $v0, 4
        la $a0, Found
        syscall

        li $v0, 1
        move $a0, $t1
        syscall

        li $t5, 1 #control2

        j next

_notfound:
        beq $t5, $t2, exit     # if the value is found exit (control == control2)

        li $v0, 1
        lw $a0, searchedVal
        syscall

        li $v0, 4
        la $a0, notFound
        syscall

        j exit

exit:
        li $v0, 4
        la $a0, newLine
        syscall
        li $v0, 30
        syscall

