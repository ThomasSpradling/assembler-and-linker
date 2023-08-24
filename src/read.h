#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "symbol-table.h"

int is_valid_label(const char* label);
int read_num(long int *output, const char *str, long int lower_bound, long int upper_bound, int bits);
int read_register(const char *str);
int write_instruction(FILE *output, const char *name, char **args, int num_args);
void write_instruction_hex(FILE *output, uint32_t instruction);
int translate_instruction(FILE *output, const char *name, char **args, size_t num_args, SymbolTable *table, SymbolTable *relocate_table, int byte_offset);