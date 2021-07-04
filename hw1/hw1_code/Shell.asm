#Can BEYAZNAR 161044038

	.data
firstMessage:	.asciiz		        "Shell.asm Executed.\n"
SelectMessage: .asciiz              "Please select your program to run.\n\n"
ShowDivisibleNumbers: .asciiz       "Press 1 to run ShowDivisibleNumbers.asm\n"
BinarySearch: .asciiz               "Press 2 to run BinarySearch.asm\n"
LinearSearch: .asciiz               "Press 3 to run LinearSearch.asm\n"
SelectionSort: .asciiz              "Press 4 to run SelectionSort.asm\n"
_quit: .asciiz                      "Press 0 to quit\n\n"
UserInput: .asciiz                  "Input : "
Table: .asciiz                      "\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n"
		
    .text

main:

    li $v0,4
    la $a0, Table
    syscall

    li $v0,4
    la $a0, firstMessage
    syscall

    li $v0,4
    la $a0, SelectMessage
    syscall

    li $v0,4
    la $a0, ShowDivisibleNumbers
    syscall

    li $v0,4
    la $a0, BinarySearch
    syscall

    li $v0,4
    la $a0, LinearSearch
    syscall

    li $v0,4
    la $a0, SelectionSort
    syscall

    li $v0,4
    la $a0, _quit
    syscall

    li $v0,4
    la $a0, UserInput
    syscall

    li $v0, 5
    syscall

    move $a0, $v0

    li $v0,18
    syscall

    j main