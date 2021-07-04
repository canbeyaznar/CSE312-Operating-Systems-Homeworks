#Can BEYAZNAR 161044038

			.data
myArray:	.word 10, 23, 37, 46, 48
searchedVal: .word		46
notFound:	.asciiz	" is not in the list."
Found:	.asciiz		" is in the list."
		.text

main_y:
		la	$a0, myArray
		la	$s0, searchedVal		
		lw	$a1, 0($s0)		
		
		jal	BinarySearch
		
		addi	$a2, $s1, 0
		jal	output	
		
		li	$v0, 10
		syscall	
		
BinarySearch:
		addi	$t1, $zero, 0	#	this will count left side 0
		addi	$t2, $zero, 4	# this will count right side n-1
		
while:		
		addi	$t3, $t1, 1
				
		bgt	$t3, $t2, breakWhile	# if left+1 > right
		beq	$t3, $t2, breakWhile	# if left+1 == right
		
		sub	$t4, $t2, $t1		# right - left
		div	$t4, $t4, 2			# (right - left) / 2
		add	$t5, $t1, $t4		# mid = left + (right - left)/2
		
		mul	$t7, $t5, 4
						
		add 	$t6, $a0, $t7		# myArray[mid] 
		lw	$t6, 0($t6)
		
		bgt	$a1, $t6, control	# if searchedVal > data[mid] jump to control
		beq	$a1, $t6, control	# if searchedVal == data[mid] jump to control
		
		add	$t2, $zero, $t5		# right = mid
		j 	while
		
control:
		add	$t1, $zero, $t5		# left = mid
		j	while
		
breakWhile:	
		mul	$t7, $t1, 4
		
		add 	$t6, $a0, $t7		# myArray[left]
		lw	$t6, 0($t6)	
		
		bne	$a1, $t6, leftSideCont	# If searchedVal != myArray[left]
		addi	$s1, $zero, 1
		j	finish
		
leftSideCont:	
		mul	$t7, $t2, 4
				
		add 	$t6, $a0, $t7		# myArray[right]
		lw	$t6, 0($t6)
		
		bne	$a1, $t6, rightSideCont	# If searchedVal != myArray[right] -> rightSideCont
		addi	$s1, $zero, 1
		j	finish
rightSideCont:		
		addi	$s1, $zero, 0

finish:	
		jr	$ra		

output:	
		li	$v0, 1
		add	$a0, $a1, 0	
		syscall
				
		bne	$a2, 0, _found
		
_notFound:		
		li	$v0, 4
		la	$a0, notFound
		syscall
		
		j	outputright

_found:
		li	$v0, 4
		la	$a0, Found
		syscall
			
outputright:
		jr	$ra