# NOTE: DEPENDS on `linker_utils.s` and `file_utils.s`

.data
base_addr:		.word 0x00400000
hex_buffer:		.space 10

.globl main
.text
#------------------------------------------------------------------------------
# function write_machine_code()
#------------------------------------------------------------------------------
# Write the contents of the .text section of the input file to the output file.
#
# Arguments:
#  $a0 = The output file pointer
#  $a1 = The input file pointer
#  $a2 = The symbol table, which you can pass into relocate_inst() if needed
#  $a3 = The relocation table, which you can pass into relocate_inst() if needed
#
# Returns: 0 on success and -1 on fail. 
#------------------------------------------------------------------------------
write_machine_code:
	# TODO

###############################################################################
# Below are in courtesy of the course CS61C GitHub Repo--This below
# is not my work.
###############################################################################

#------------------------------------------------------------------------------
# function build_tables()
#------------------------------------------------------------------------------
# Build tables
# $a0 = input file arr, $a1 = input arr len, $a2 = global offset
# $v0 = symbol table, $v1 = reloc_data
#------------------------------------------------------------------------------
build_tables:
	addiu $sp, $sp, -32		# Begin build_tables()
	sw $s0, 28($sp)
	sw $s1, 24($sp)
	sw $s2, 20($sp)
	sw $s3, 16($sp)
	sw $s4, 12($sp)
	sw $s5, 8($sp)
	sw $s6, 4($sp)
	sw $ra, 0($sp)
	# Store arguments:
	move $s0, $a0	# $s0 = input file array
	move $s1, $a1	# $s1 = number of files
	move $s2, $a2	# $s2 = base offset
	# Open files:
	jal open_files
	move $s3, $v0	# $s3 = file descriptors
	# Alloc space for reloc_data
	sll $a0, $s1, 3	# 8 * (# files)
	li $v0, 9
	syscall
	move $s4, $v0	# $s4 = array of reloc_data
	# Loop through and build table:
	li $s5, 0		# $s5 = counter
	li $s6, 0		# $s6 = symbol table (empty)
build_tables_loop:
	beq $s5, $s1, build_tables_end
	
	sll $t0, $s5, 2	
	addu $t5, $s3, $t0	# $a0 = current file handle
	lw $a0, 0($t5)
	move $a1, $s6	# $a1 = current symobl table
	sll $t1, $t0, 1	
	addu $a2, $s4, $t1	# $a2 = current entry in reloc_data
	move $a3, $s2	# $a3 = current global offset
	jal fill_data
	blt $v0, $0, build_tables_error
	move $s6, $v1	# update symbol table
	sll $t1, $s5, 3	
	addu $t2, $s4, $t1	# current entry in reloc_data
	lw $t3, 0($t2)
	addu $s2, $s2, $t3	# update offset
	addiu $s5, $s5, 1	# increment count
	j build_tables_loop
build_tables_error:
	move $a0, $s3
	move $a1, $s1
	jal close_files
	la $a0, error_parsing
	li $v0, 4
	syscall
	li $a0, 1
	li $v0, 17		# exit on error
	syscall
build_tables_end:
	move $a0, $s3
	move $a1, $s1
	jal close_files
	# Set return values
	move $v0, $s6	# set symbol table
	move $v1, $s4	# set reloc_data
	# Stack stuff
	lw $s0, 28($sp)
	lw $s1, 24($sp)
	lw $s2, 20($sp)
	lw $s3, 16($sp)
	lw $s4, 12($sp)
	lw $s5, 8($sp)
	lw $s6, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 32
	jr $ra			# End build_tables()


#------------------------------------------------------------------------------
# function main()
#------------------------------------------------------------------------------
# argc = num args, argv = char**
#------------------------------------------------------------------------------
main:	bge $a0, 2, m_arg_ok
	# Error not enough arguments
	la $a0, error_args
	li $v0, 4
	syscall
	li $a0, 1	
	li $v0, 17
	syscall			# exit with error
m_arg_ok:	
	addiu $s0, $a0, -1		# $s0 = num inputs
	move $s1, $a1		# $s1 = array of filenames
	sll $t0, $s0, 2
	addu $t1, $s1, $t0		
	lw $s2, 0($t1)		# $s2 = output filename
	
	# Build symbol/reloc table:
	move $a0, $s1
	move $a1, $s0
	la $t0, base_addr
	lw $a2, 0($t0)
	jal build_tables
	move $s3, $v0		# $s3 = symbol table
	move $s4, $v1		# $s4 = reloc_data
	
	# Open input & output files for writing:
	move $a0, $s1
	move $a1, $s0
	jal open_files
	move $s1, $v0		# $s1 = array of file ptrs
	move $a0, $s2
	li $a1, 1
	li $v0, 13
	syscall			# open output file
	blt $v0, 0, m_fopen_error
	move $s2, $v0		# $s2 = output file descriptor

	# Begin writing:
	li $s5, 0
m_write_loop:
	beq $s5, $s0, m_write_done
	move $a0, $s2		# $a0 = output file
	sll $t0, $s5, 2		# sizeof(file_ptr) = 4
	addu $t1, $s1, $t0
	lw $a1, 0($t1)		# $a1 = input file
	move $a2, $s3		# $a2 = symbol table
	sll $t0, $s5, 3		# sizeof(reloc_data) = 8
	addu $t2, $s4, $t0
	lw $a3, 4($t2)		# $a3 = reloc table
	jal write_machine_code
	blt $v0, $0, m_write_error
	addiu $s5, $s5, 1
	j m_write_loop
m_write_done:
	move $a0, $s1
	move $a1, $s0
	jal close_files
	move $a0, $s2
	li $v0, 16
	syscall
	la $a0, link_success
	li $v0, 4
	syscall
	li $v0, 10
	syscall			# exit without errors
m_write_error:
	move $a0, $s1
	move $a1, $s0
	jal close_files
	move $a0, $s2
	li $v0, 16
	syscall
	la $a0, error_write
	li $v0, 4
	syscall
	li $a0, 1
	li $v0, 17
	syscall			# exit with errors
m_fopen_error:
	move $a0, $s1
	move $a1, $s0
	jal close_files
	la $a0, error_fopen
	li $v0, 4
	syscall
	li $a0, 1
	li $v0, 17
	syscall			# exit with errors

.data
error_args:		.asciiz "Error not enough arguments. Exiting.\n"
error_parsing:	.asciiz "Error parsing files. Exiting.\n"
error_fopen:	.asciiz "Error opening output file. Exiting.\n"
error_write:	.asciiz "Error during write. Exiting.\n"
link_success:	.asciiz "Operation completed successfully.\n"