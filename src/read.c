#include "read.h"

// Checks if the `label` has a valid name.
int is_valid_label(const char* label) {
    if (!label) {
        return 0;
    }

    int first = 1;
    while (*label) {
        if (first) {
            if (!isalpha((int) *label) && *label != '_') {
                return 0;
            } else {
                first = 0;
            }
        } else if (!isalnum((int) *label) && *label != '_') {
            return 0;
        }
        label++;
    }
    return first ? 0 : 1;
}

// Converts a string to a long.
int read_num(long *output, const char *str, long lower_bound, long upper_bound, int bits) {
    if (!str || !output || bits <= 0 || bits > 32) {
        return -1;
    }
    
    long num = strtol(str, NULL, 0);
    if (num == 0L) {
        return -1;
    }
    
    long max_val = (1L << (bits - 1)) - 1;
    long min_val = -(1L << (bits - 1));
    
    if (num > max_val && num <= (1L << bits) - 1) {
        num -= (1L << bits);
    }
    
    if (min_val <= num && num <= max_val && lower_bound <= num && num <= upper_bound) {
        *output = (int) num;
        return 0;
    }
    return -1;
}

// Takes in a register name and produces the the corresponding register number.
int read_register(const char *str) {
    if (strcmp(str, "$zero") == 0 || strcmp(str, "$0") == 0) return 0;
    else if (strcmp(str, "$at") == 0) return 1;
    else if (strcmp(str, "$v0") == 0) return 2;

    else if (strcmp(str, "$a0") == 0) return 4;
    else if (strcmp(str, "$a1") == 0) return 5;
    else if (strcmp(str, "$a2") == 0) return 6;
    else if (strcmp(str, "$a3") == 0) return 7;

    else if (strcmp(str, "$t0") == 0) return 8;
    else if (strcmp(str, "$t1") == 0) return 9;
    else if (strcmp(str, "$t2") == 0) return 10;
    else if (strcmp(str, "$t3") == 0) return 11;

    else if (strcmp(str, "$s0") == 0) return 16;
    else if (strcmp(str, "$s1") == 0) return 17;
    else if (strcmp(str, "$s2") == 0) return 18;
    else if (strcmp(str, "$s3") == 0) return 19;

    else if (strcmp(str, "$sp") == 0) return 29;
    else if (strcmp(str, "$ra") == 0) return 31;
    else return -1;
}

// Prints instruction into output in hex format.
void write_instruction_hex(FILE *output, uint32_t instruction) {
    fprintf(output, "%08x\n", instruction);
}

/**************************
 * MIPS Write Format Types
 **************************/
//
// Here MIPS supports three formatting types: R, I, and J. The below displays the three methods for parsing these
// along with a standard MIPS architecture formatting over theses types.
//

/** `write_rtype` class commits an operation of the form:
 *     opcode   rs     rt     rd    shamt  funct
 *    [  6   ][  5  ][  5  ][  5  ][  5  ][  6   ]
*/
static int write_rtype(int opcode, char *rs, char *rt, char *rd, char *shamt, int funct, FILE *output, size_t num_args, size_t expected_args) {
    if (num_args != expected_args) {
        return 1;
    }
    int rs_num = read_register(rs);
    int rt_num = read_register(rt);
    int rd_num = read_register(rd);
    if (rs_num == -1 || rt_num == -1 || rd_num == -1) {
        return 1;
    }
    long shamt_num;
    if (read_num(&shamt_num, shamt, -(1<<4), (1<<4) - 1, 5) == -1) {
        return 1;
    }

    uint32_t instruction = funct;
    // Mask shift amount with 0001 1111
    instruction += (shamt_num & 0x1F) << 6;
    instruction += rd_num << (6 + 5);
    instruction += rt_num << (6 + 5 + 5);
    instruction += rs_num << (6 + 5 * 3);
    instruction += opcode << (6 + 5 * 4);
    write_instruction_hex(output, instruction);
    return 0;
}

/** `write_immediate_itype` class commits an operation of the form:
 *     opcode   rs     rt        immediate
 *    [  6   ][  5  ][  5  ][       16       ]
*/
static int write_immediate_itype(int opcode, char *rs, char *rt, char *immediate, FILE *output, size_t num_args, size_t expected_args) {
    if (num_args != expected_args) {
        return 1;
    }
    int rs_num = read_register(rs);
    int rt_num = read_register(rt);
    if (rs_num == -1 || rt_num == -1) {
        return 1;
    }

    long immediate_num;
    if (read_num(&immediate_num, immediate, -(1<<15), (1<<15) - 1, 16) == -1) {
        return 1;
    };

    uint32_t instruction = (uint32_t) (immediate_num & 0xFFFF);
    instruction += rt_num << 16;
    instruction += rs_num << (16 + 5);
    instruction += opcode << (16 + 5 + 5);

    write_instruction_hex(output, instruction);
    return 0;
}

/** `write_branch_itype` class commits an operation of the form:
 *     opcode   rs     rt        address
 *    [  6   ][  5  ][  5  ][       16       ]
*/
static int write_branch_itype(int opcode, char *rs, char *rt, char *label, FILE *output, size_t num_args, size_t expected_args, SymbolTable *table, SymbolTable *relocate_table, int byte_offset) {
    if (num_args != expected_args) {
        return 1;
    }
    // ensure offset is divisible by 4
    if (byte_offset & 0x3) {
        return 1;
    }

    int rs_num = read_register(rs);
    int rt_num = read_register(rt);
    if (rs_num == -1 || rt_num == -1) {
        return 1;
    }

    int32_t address_num = get_from_table(table, label);

    // Ensure label exists
    if (get_from_table(table, label) == -1) {
        return 1;
    }

    int32_t offset = address_num - (byte_offset + 4);

    // convert to instruction offset
    offset >>= 2;

    if (offset < -(1<<15) || offset > (1<<15) - 1) {
        return 1;
    }

    uint32_t instruction = offset & 0xFFFF;
    instruction += rt_num << 16;
    instruction += rs_num << (16 + 5);
    instruction += opcode << (16 + 5 + 5);
    write_instruction_hex(output, instruction);
    return 0;
}

/** `write_jtype` class commits an operation of the form:
 *     opcode           address
 *    [  6   ][            26            ]
*/
static int write_jtype(int opcode, char *label, FILE *output, size_t num_args, size_t expected_args, SymbolTable *table, SymbolTable *relocate_table, int byte_offset) {
    if (num_args != expected_args) {
        return 1;
    }
    // ensure offset is divisible by 4
    if (byte_offset & 0x3) {
        return 1;
    }
    if (!is_valid_label(label)) {
        return 1;
    }

    // Ensure label exists
    if (get_from_table(table, label) == -1) {
        return 1;
    }

    add_to_table(relocate_table, label, byte_offset);

    uint32_t instruction = 0;
    instruction += opcode << 26;
    write_instruction_hex(output, instruction);
    return 0;
}

// Translates an instruction into a hexadecimal format.
int translate_instruction(FILE *output, const char *name, char **args, size_t num_args, SymbolTable *table, SymbolTable *relocate_table, int byte_offset) {
    // Add Unsigned: `addu $rd, $rs, $rt`
    if (strcmp(name, "addu") == 0) return write_rtype(0x00, args[1], args[2], args[0], "0", 0x21, output, num_args, 3);
    
    // Or: `or $rd, $rs, $rt`
    else if (strcmp(name, "or") == 0) return write_rtype(0x00, args[1], args[2], args[0], "0", 0x25, output, num_args, 3);

    // Set Less Than: `slt $rd, $rs, $rt`
    else if (strcmp(name, "slt") == 0) return write_rtype(0x00, args[1], args[2], args[0], "0", 0x2a, output, num_args, 3);

    // Set Less Than Unsigned: `sltu $rd, $rs, $rt`
    else if (strcmp(name, "sltu") == 0) return write_rtype(0x00, args[1], args[2], args[0], "0", 0x2b, output, num_args, 3);

    // Jump Register: `jr $rs`
    else if (strcmp(name, "jr") == 0) return write_rtype(0x00, args[0], "$0", "$0", "0", 0x08, output, num_args, 1);

    // Shift Left Logical: `sll $rd, $rt, shamt`
    else if (strcmp(name, "sll") == 0) return write_rtype(0x00, "$0", args[1], args[0], args[2], 0x0, output, num_args, 3);

    // Add Immediate Unsigned: `addiu $rt, $rs, immediate`
    else if (strcmp(name, "addiu") == 0) return write_immediate_itype(0x09, args[1], args[0], args[2], output, num_args, 3);

    // Or Immediate: `ori $rt, $rs, immediate`
    else if (strcmp(name, "ori") == 0) return write_immediate_itype(0x0d, args[1], args[0], args[2], output, num_args, 3);

    // Load Upper Immediate: `lui $rt, immediate`
    else if (strcmp(name, "lui") == 0) return write_immediate_itype(0x0f, "$0", args[0], args[1], output, num_args, 2);

    // Load Byte: `lb $rt, offset($rs)`
    else if (strcmp(name, "lb") == 0) return write_immediate_itype(0x20, args[2], args[0], args[1], output, num_args, 3);

    // Load Byte Unsigned: `lbu $rt, offset($rs)`
    else if (strcmp(name, "lbu") == 0) return write_immediate_itype(0x24, args[2], args[0], args[1], output, num_args, 3);

    // Load Word: `lw $rt, offset($rs)`
    else if (strcmp(name, "lw") == 0) return write_immediate_itype(0x23, args[2], args[0], args[1], output, num_args, 3);

    // Store Byte: `sb $rt, offset($rs)`
    else if (strcmp(name, "sb") == 0) return write_immediate_itype(0x28, args[2], args[0], args[1], output, num_args, 3);

    // Store Word: `sw $rt, offset($rs)`
    else if (strcmp(name, "sw") == 0) return write_immediate_itype(0x2b, args[2], args[0], args[1], output, num_args, 3);

    // Branch on Equal: `beq $rs, $rt, label`
    else if (strcmp(name, "beq") == 0) return write_branch_itype(0x04, args[0], args[1], args[2], output, num_args, 3, table, relocate_table, byte_offset);

    // Branch on Not Equal: `bne $rs, $rt, label`
    else if (strcmp(name, "bne") == 0) return write_branch_itype(0x05, args[0], args[1], args[2], output, num_args, 3, table, relocate_table, byte_offset);

    // Jump: `j label`
    else if (strcmp(name, "j") == 0) return write_jtype(0x02, args[0], output, num_args, 1, table, relocate_table, byte_offset);

    // Jump and Link: `jal label`
    else if (strcmp(name, "jal") == 0) return write_jtype(0x03, args[0], output, num_args, 1, table, relocate_table, byte_offset);

    else return -1;
}

// Writes an instruction into the .int file and returns the number of instructions written.
int write_instruction(FILE *output, const char *name, char **args, int num_args) {
    // Load Immediate pseudoinstruction: `li $rt, immediate`
    if (strcmp(name, "li") == 0) {
        int str_num = strtol(args[1], NULL, 0);

        if (-(1<<15) <= str_num && str_num <= (1<<15) - 1) {
            fprintf(output, "addiu %s %s %s\n", args[0], "$0", args[1]);
            return 1;
        }
        if (str_num < -(1L<<31) && str_num >= (1L<<31) - 1) {
            return 0;
        }
        // Here ((1 << 16) - 1) is a 16 ones
        int upper = (str_num >> 16) & ((1 << 16) - 1);
        int lower = str_num & ((1 << 16) - 1);
        fprintf(output, "lui $at 0x%x\n", upper);
        fprintf(output, "ori %s $at 0x%x\n", args[0], lower);
        return 2;
    }
    // Branch on Less Than pseudoinstruction: `blt $rs, $rt, label`
    else if (strcmp(name, "blt") == 0) {
        fprintf(output, "slt $at %s %s\n", args[0], args[1]);
        fprintf(output, "bne $at $0 %s\n", args[2]);
        return 2;
    } else {
        fprintf(output, "%s", name);
        for (int i = 0; i < num_args; i++) {
            fprintf(output, " %s", args[i]);
        }
        fprintf(output, "\n");
        return 1;
    }
}