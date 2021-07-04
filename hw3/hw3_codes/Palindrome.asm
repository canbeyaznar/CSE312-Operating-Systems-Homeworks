# Can BEYAZNAR
# 161044038

.data

palindrome_msg:	        .asciiz	    ": Palindrome"
notpalindrome_msg:	    .asciiz		": Not Palindrome"
loopend_msg:	        .asciiz		"\nLOOP END\n"
continue_msg:           .asciiz     "Do you want to continue (y/n)?\n"
goodbye_msg:            .asciiz     "\nGoodbye…\n"
enterlast_msg:          .asciiz     "\nPlease enter the last word:\n"

twoDot_msg:             .asciiz ": "
new_line:               .asciiz     "\n"
startindex_msg:         .asciiz "start : "
endindex_msg:           .asciiz "end : "

# This is palindrome dictionary 
# that contains 100 words, where 90 of the words are not palindrome
dictionary: .asciiz "aba,azbza,ada,dontnod,tacocat,madam,kayak,refer,toyot,bob,scrub,sack,itchy,check,noxious,tie,phobic,hungry,songs,mammoth,science,amount,middle,tiger,burn,dispensable,beef,dime,subdued,program,prick,outstanding,deafening,queue,shoe,stove,uttermost,harass,deserve,wise,direction,underwear,card,unbecoming,key,curtain,war,stomach,bait,loutish,automatic,start,fallacious,song,float,seemly,yarn,remain,guitar,quirky,odd,condition,shape,dizzy,degree,cats,substantial,adhesive,magic,introduce,paint,stain,damage,curve,nonstop,fog,known,beg,expansion,magnificent,shoes,bloody,decision,robust,bead,quilt,satisfying,young,glass,rescue,poised,glamorous,heal,tested,touch,wobble,request,chop,contain,stroke,\n"
user_input:	.space		20
cont_input: .space      4

.text

main_m: 

    addi $s1, $0, 0 # start index
    addi $s2, $0, 0 # end index
    addi $s4, $0, 1 # this will count the number of readed word

    addi $sp, $sp, -4000 # allocate 240 bytes

    la $s3, dictionary
    sw $s3, ($sp)
    addi $t0, $0, 0

    
    get_string_size:
        

        lb $t1, ($s3)
        
        beq $t1, 44, control_each_word # next word will come
        beq $t1, 10, exit # all words are readed

        addi $s3, $s3, 1 # go to the next character
        addi $s2, $s2, 1 # increase end index
        addi $t0, $t0, 1
        j get_string_size

    control_each_word:
        
        #sub $s2, $s2, 1

        li $v0,20
        syscall

        li $v0, 1
        addi $a0, $s4, 0
        syscall

        li $v0, 4
        la $a0, twoDot_msg
        syscall

        sub	$s5, $s2, $s1 # get size of word
        addi $t6, $0, 0

        lw $a1, ($sp)
        add $a1, $a1, $s1
        print_str:
            beq $t6, $s5, continue_control

            li $v0, 11
            lb $a0, ($a1)
            syscall
            
            addi $a1, $a1, 1
            addi $t6, $t6, 1

            j print_str

        continue_control:

            addi $s3, $s3, 1
            addi $s4, $s4, 1
        
            j controlTheSize

        updateIndexes:
            addi $s2, $s2, 1
            addi $s1, $s2, 0 # assign new start index for another word
            lw $s3, ($sp)
            add $s3, $s3, $s2

            li $v0,20
            syscall
            
            j get_string_size

        
        #j get_string_size

    # it will compare the character byte by byte
    # one register will start from begining other one
    # will start from the end of word
    compare_bytes:
        
        beq $t5, $t6, palindrome

        lb $t3, ($a1)   # get byte from left side
        lb $t4, ($s3)   # get byte from right side       

        bne $t3, $t4, not_palindrome

        sub $s3, $s3,1       # turn back 1 index
        addi $a1, $a1, 1     # go forward 1 index

        addi $t5, $t5, 1     # increase the $t0 value
                             # this parameter will control current index of string
        j compare_bytes

    controlTheSize:
        addi    $t2, $0,2
        div		$s5, $t2			# $t0 / $t1
        mflo	$t6					# this will keep middle index of string # $t6 = floor($t0 / $t1) 
        mfhi	$t4					# $t4 = $t0 mod $t1 

        beq $t4,0,not_palindrome    # if the length is even 
                                    # print not palindrome
        
        lw $a1, ($sp)   # assign two points to its start location 
        lw $s3, ($sp)
        add $a1, $a1, $s1   # start of word
        add $s3, $s3, $s2   # end of word
        sub $s3, $s3, 1
        addi $t5, $0, 0             # this will count the current index of word 
        
        j compare_bytes # if it is odd control the word it is palindrome or not

exit:

    addi $sp, $sp, 4000

    li $v0,20   # ALWAYS WAIT FOR INPUT PROGRAM CAN WORK WRONG!!
    syscall

    li $v0, 4
    la $a0, continue_msg
    syscall

    li $v0, 8
    la $a0, cont_input  # take input from user
    la $a1, 4
    syscall

    lb $a1, ($a0)
    beq $a1, 121, takeinput # if input is 'y' take input for palindrome

    
    _exit_:

        li $v0, 4
        la $a0, goodbye_msg
        syscall

        li $v0, 30
        syscall


    takeinput:
        li $v0, 4
        la $a0, enterlast_msg
        syscall

        li $v0, 8
        la $a0, user_input
        li $a1, 20      #   max size must be 20
        syscall

    
        addi $sp, $sp, -20 # allocate 20 bytes

        sw $a0, ($sp)
        lw $a1, ($sp)

        li $v0, 1
        addi $a0, $s4,0
        syscall

        li $v0, 4
        la $a0, twoDot_msg
        syscall

        addi $t0, $0, 0

        get_string_size_i:
            lb $t1, ($a1)
            
            beq $t1, 10, controlTheSize_i
            
            li $v0, 11
            lb $a0, ($a1)
            syscall

            addi $a1, $a1, 1 # go to the next character
            addi $t0, $t0, 1 # count the length
            j get_string_size_i
        
        
        li $v0, 30
        syscall

controlTheSize_i:

        addi    $t2, $0,2
        div		$t0, $t2			# $t0 / $t1
        mflo	$t6					# this will keep middle index of string # $t6 = floor($t0 / $t1) 
        mfhi	$t4					# $t4 = $t0 mod $t1 

        beq $t4,0,not_palindrome_i  # if the length is even 
                                    # print not palindrome
    
    
    sub $a1, $a1, 1 # turn back 1 index
    lw $s2, ($sp)   # go back to the first byte

    addi $t0, $0, 0

    compare_bytes_i:

        beq $t0, $t6, palindrome_i  # If t0 is in the middle of the string and other 
                                    # characters are symmetrical, print it is palindrome

        lb $t3, ($a1)
        lb $t4, ($s2)
        bne $t3, $t4, not_palindrome_i

        
        sub  $a1, $a1, 1     # turn back 1 index
        addi $s2, $s2, 1     # go forward 1 index
        addi $t0, $t0, 1     # increase the $t0 value
                             # this parameter will control current index of string


        j compare_bytes_i

    li $v0, 30
    syscall
     

not_palindrome:

    li $v0,4
    la $a0, notpalindrome_msg
    syscall

    li $v0,4
    la $a0, new_line
    syscall

    j updateIndexes

palindrome:

    li $v0,4
    la $a0, palindrome_msg
    syscall

    li $v0,4
    la $a0, new_line
    syscall

    j updateIndexes

not_palindrome_i:
    li $v0,4
    la $a0, notpalindrome_msg
    syscall

    li $v0,4
    la $a0, new_line
    syscall

    addi $sp, $sp, 20

    j _exit_

palindrome_i:

    li $v0,4
    la $a0, palindrome_msg
    syscall

    li $v0,4
    la $a0, new_line
    syscall

    addi $sp, $sp, 20

    j _exit_