.text	

#------------------------------------------------------------------------------
# function hex_to_str()
#------------------------------------------------------------------------------
# Writes a 32-bit number in hexadecimal format to a string buffer, followed by
# the newline character and the NUL-terminator.
#
# For example:
#  0xabcd1234 => "abcd1234\n\0"
#  0x134565FF => "134565ff\n\0"
#  0x38       => "00000038\n\0"
#
# Arguments:
#  $a0 = int to write
#  $a1 = character buffer to write into
#
# Returns: none
#------------------------------------------------------------------------------
hex_to_str:
    # Save registers
    addi $sp, $sp, -12
    sw $ra, 0($sp)
    sw $s0, 4($sp)
    sw $s1, 8($sp)
    
    # Initialize constants and buffer settings
    li $s0, 7       # i = 7
    sb $zero, 9($a1)  # res[9] = \0
    li $t0, 0x0A     # newline ASCII value
    sb $t0, 8($a1)  # res[8] = \n
    add $s1, $a1, $s0  # s1 points to the current position in buffer (starts at res[7])

hex_loop:
    # Extract the 4 least significant bits
    andi $t1, $a0, 0xF  # curr = n & 0xF

    # Convert 4-bit number to letter
    jal ascii_converter_hex
    sb $v0, 0($s1)  # store result in buffer

    # Update for the next iteration
    srl $a0, $a0, 4  # n = n >> 4
    subi $s0, $s0, 1  # i--
    subi $s1, $s1, 1  # move to the previous position in buffer

    bge $s0, $zero, hex_loop  # continue looping while i >= 0

    # Restore registers and return
    lw $ra, 0($sp)
    lw $s0, 4($sp)
    lw $s1, 8($sp)
    addi $sp, $sp, 12
    jr $ra

# Function f() to convert 4-bit number to its corresponding hex character
ascii_converter_hex:
    li $t2, 9
    ble $a0, $t2, is_digit

    # Convert to alphabet (lowercase a-f)
    addi $a0, $a0, 87  # 10 -> 'a', 11 -> 'b', ..., 15 -> 'f'
    move $v0, $a0
    jr $ra

is_digit:
    # Convert to number (0-9)
    addi $a0, $a0, 48  # 0 -> '0', 1 -> '1', ..., 9 -> '9'
    move $v0, $a0
    jr $ra

###############################################################################
# Below are in courtesy of the course CS61C GitHub Repo--This below
# is not my work.
###############################################################################

#------------------------------------------------------------------------------
# function parse_int()
#------------------------------------------------------------------------------
# Parses the string as an unsigned integer. The only bases supported are 10 and
# 16. We will assume that the number is valid, and that overflow does not happen.
#
# Arguments: 
#  $a0 = string containing a number
#  $a1 = base (will be either 10 or 16)
#
# Returns: the number
#------------------------------------------------------------------------------
parse_int:
	li $v0, 0				# Begin parse_int()
	li $t1, 'A'
	li $t2, 'F'
parse_int_loop:
	lb $t0, 0($a0)
	beq $t0, $0, parse_int_done
	mul $v0, $v0, $a1		# multiply by the base
	blt $t0, $t1, parse_int_dec
	ble $t0, $t2, parse_int_hex_upper
	# parse as lowercase hex:
	addiu $t0, $t0, -87		# 'a' - 10
	j parse_int_common
parse_int_dec:
	addiu $t0, $t0, -48		# 0 - '0'
	j parse_int_common
parse_int_hex_upper:
	addiu $t0, $t0, -55		# 'A' - 10
parse_int_common:
	addu $v0, $v0, $t0
	addiu $a0, $a0, 1
	j parse_int_loop
parse_int_done:
	jr $ra				# End parse_int()

#------------------------------------------------------------------------------
# function tokenize()
#------------------------------------------------------------------------------
# Converts a line of symbol/relocation table output into a numerical address
# and a name string, which can then be added into the appropriate SymbolList
# after you add the appropriate offset to the addr. Note that this function
# returns TWO values. This is just a shortcut to make the MIPS shorter.
#
# Arguments:
#  $a0 = string containing a line from symbol/relocation table output
#
# Returns: 	$v0: the address of the symbol
#	$v1: name of the string
#------------------------------------------------------------------------------	
tokenize:
	addiu $sp, $sp, -20			# Begin tokenize()
	sw $s0, 16($sp)
	sw $s1, 12($sp)
	sw $s2, 8($sp)
	sw $s3, 4($sp)
	sw $ra, 0($sp)
	move $s0, $a0		# $s0 = start of string
	li $s1, 0			# $s1 = offset from start
	li $s2, 9			# $s2 = tab character
tokenize_loop:
	addu $t0, $s0, $s1
	lb $t1, 0($t0)
	beq $t1, $0, tokenize_fail	
	beq $t1, $s2, tokenize_done
	addiu $s1, $s1, 1
	j tokenize_loop
tokenize_done:
	sb $0, 0($t0)
	li $a1, 10
	jal parse_int		# $v0 = int result
	addu $v1, $s0, $s1
	addiu $v1, $v1, 1		# $v1 = beginning of string
tokenize_fail: # fallthrough (we should never directly jump to here unless input is invalid)
	lw $s0, 16($sp)
	lw $s1, 12($sp)
	lw $s2, 8($sp)
	lw $s3, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 20
	jr $ra				# End tokenize()

#------------------------------------------------------------------------------
# function readline()
#------------------------------------------------------------------------------
# Reads the next line from the file until a newline or end of file is reached.
# If a newline was reached, the last newline character is discarded. A NUL-
# terminator is added to the end of the string read. 
#
# The buffer is returned in $v1. It is VOLATILE - the next time readline() is
# called, the buffer will be overwritten. This should be fine for most linker
# functions except for the symbol/relocation table.
#
# Arguments:	
#  $a0 = File handle
#
# Returns: 	$v0: the # of bytes read, or -1 if error and nothing was read
#	$v1: pointer to buffer with chars read
#------------------------------------------------------------------------------
readline:	
	la $t0, buffer			# Begin readline()
	li $t1, 512
	la $a1, buffer
	li $a2, 1
readline_next:
	li $v0, 14			# read until a newline is reached
	syscall			
	beq $v0, $0, readline_done	# only end-of-file is left, nothing was read
	blt $v0, $0, readline_err	# error, nothing was read
	lbu $t2, 0($a1)
	li $t3, 0x0a		# newline char
	beq $t2, $t3, readline_done
	addiu $a1, $a1, 1
	subu $t4, $a1, $t0
	bgt $t4, $t1, readline_err2
	j readline_next
readline_done:			# at this point, $v0 contains the # bytes read
	sb $0, 0($a1)
	move $v1, $t0
	jr $ra
readline_err:
	la $a0, readline_err_syscall
	li $v0, 4
	syscall
	li $v0, -1
	jr $ra
readline_err2:
	la $a0, readline_err_bufsize
	li $v0, 4
	syscall
	li $v0, -1
	jr $ra				# End readline()

.data
buffer:	.space 1024
.data
readline_err_syscall:
	.asciiz "Error in readline: Could not read from file.\n"
readline_err_bufsize:
	.asciiz "Error in readline: Exceeded maximum buffer size.\n"