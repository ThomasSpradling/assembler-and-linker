.data
newline:	.asciiz "\n"
tab:	.asciiz "\t"

.text
#------------------------------------------------------------------------------
# function strlen()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string input
#
# Returns: the length of the string
#------------------------------------------------------------------------------
strlen:
  addi $sp, $sp, -4
  sw $a0, 0($sp)
  li $t0, 0

strlen_loop:
  lb $t2, 0($a0)
  beq $t2, 0, strlen_exit # if A[current] == 0, exit loop
  addi $t0, $t0, 1 # i++
  addi $a0, $a0, 1 # current++
  j strlen_loop

strlen_exit:
  move $v0, $t0 # set return value as i

  lw $a0, 0($sp)
  addi $sp, $sp, 4
  jr $ra

#------------------------------------------------------------------------------
# function strncpy()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = pointer to destination array (A)
#  $a1 = source string (s)
#  $a2 = number of characters to copy (n)
#
# Returns: the destination array
#------------------------------------------------------------------------------
strncpy:
  # push into stack
  addi $sp, $sp, -12
  sw $a0, 0($sp)
  sw $a1, 4($sp)
  sw $a2, 8($sp)

  li $t0, 0 # i = 0
strncpy_loop:
  # exit loop if i = n
  beq $t0, $a2, strncpy_exit

  # exit loop if s[i] = 0
  add $t2, $a1, $t0
  lb $t1, 0($t2)  # s[i]
  beq $t1, $0, strncpy_exit

  add $t3, $a0, $t0
  sb $t1, 0($t3) # A[i] = s[i]

  addi $t0, $t0, 1  # i++

  j strncpy_loop

strncpy_exit:
  move $v0, $a0
  # pop from stack
  lw $a0, 0($sp)
  lw $a1, 4($sp)
  lw $a2, 8($sp)
  addi $sp, $sp, 12

  jr $ra

#------------------------------------------------------------------------------
# function copy_of_str()
#------------------------------------------------------------------------------
# Creates a copy of a string.
#
# Arguments:
#   $a0 = string to copy (s)
#
# Returns: pointer to the copy of the string
#------------------------------------------------------------------------------
copy_of_str:
    # Push registers to the stack
    addi $sp, $sp, -16
    sw $ra, 0($sp)
    sw $s0, 4($sp)
    sw $s1, 8($sp)
    sw $s2, 12($sp)

    # Store the string address
    move $s1, $a0

    # Call strlen to get string length
    jal strlen
    move $s2, $v0

    # Allocate memory (length + 1 for the null terminator)
    addi $a0, $s2, 1
    li $v0, 9
    syscall

    # Store the start address of the new memory space
    move $s0, $v0

    # Call strncpy
    move $a0, $s0
    move $a1, $s1
    move $a2, $s2
    jal strncpy

    # Set the return value
    move $v0, $s1

    # Restore registers and return
    lw $ra, 0($sp)
    lw $s0, 4($sp)
    lw $s1, 8($sp)
    lw $s2, 12($sp)
    addi $sp, $sp, 16
    jr $ra

###############################################################################
# Below are in courtesy of the course CS61C GitHub Repo--This below
# is not my work.
###############################################################################

#------------------------------------------------------------------------------
# function streq()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string 1
#  $a1 = string 2
#
# Returns: 0 if string 1 and string 2 are equal, -1 if they are not equal
#------------------------------------------------------------------------------
streq:
	beq $a0, $0, streq_false	# Begin streq()
	beq $a1, $0, streq_false
streq_loop:
	lb $t0, 0($a0)
	lb $t1, 0($a1)
	addiu $a0, $a0, 1
	addiu $a1, $a1, 1
	bne $t0, $t1, streq_false
	beq $t0, $0, streq_true
	j streq_loop
streq_true:
	li $v0, 0
	jr $ra
streq_false:
	li $v0, -1
	jr $ra			# End streq()

#------------------------------------------------------------------------------
# function dec_to_str()
#------------------------------------------------------------------------------
# Convert a number to its unsigned decimal integer string representation, eg.
# 35 => "35", 1024 => "1024". 
#
# Arguments:
#  $a0 = int to write
#  $a1 = character buffer to write into
#
# Returns: the number of digits written
#------------------------------------------------------------------------------
dec_to_str:
	li $t0, 10			# Begin dec_to_str()
	li $v0, 0
dec_to_str_largest_divisor:
	div $a0, $t0
	mflo $t1		# Quotient
	beq $t1, $0, dec_to_str_next
	mul $t0, $t0, 10
	j dec_to_str_largest_divisor
dec_to_str_next:
	mfhi $t2		# Remainder
dec_to_str_write:
	div $t0, $t0, 10	# Largest divisible amount
	div $t2, $t0
	mflo $t3		# extract digit to write
	addiu $t3, $t3, 48	# convert num -> ASCII
	sb $t3, 0($a1)
	addiu $a1, $a1, 1
	addiu $v0, $v0, 1
	mfhi $t2		# setup for next round
	bne $t2, $0, dec_to_str_write
	jr $ra			# End dec_to_str()
  