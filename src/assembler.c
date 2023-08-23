#include "assembler.h"

// Reads filename `f` and assembles it in two-passes.
//  (1)   The first pass will strip any comments and generate a symbol table
// all into an intermediate .int file. After this pass is complete, the file `f` will
// be closed.
//  (2)   The second pass will read this .int file and produce the .out file
// from this cleaned up assembly file.
//
//  -     Afterward, the .int file will be removed.
int assemble(char *input_file, char *output_file) {
    FILE *input = fopen(input_file, "r");
    if (input == NULL) {
        fprintf(stderr, "ERROR: Failed to open file %s.\n", input_file);
        return 3;
    }

    FILE *intermediate = fopen(output_file, "w");
    if (intermediate == NULL) {
        fprintf(stderr, "ERROR: Failed to open intermediate file %s.\n", output_file);
        return 3;
    }

    int line_number = -1;
    int byte_offset = 0;
    SymbolTable *table = create_table();

    char line[100];
    while (fgets(line, sizeof(line), input) != NULL) {
        line_number++;

        char *comment = strchr(line, '#');
        if (comment != NULL) {
            *comment = '\0';
        }

        char *label_end = strchr(line, ':');
        if (label_end != NULL) {
            *label_end = '\0';
            add_to_table(table, line, byte_offset);
            memmove(line, label_end + 1, strlen(label_end + 1) + 1);
        }

        char *name = strtok(line, " \t\n");
        if (name == NULL) {
            continue;
        }

        char *args[3];
        int num_args = 0;
        char *current;
        while ((current = strtok(NULL, ", \t\n()")) != NULL && num_args < 3) {
            args[num_args] = current;
            num_args++;
        }

        byte_offset += 4 * write_instruction(intermediate, name, args, num_args);
    }

    free_table(table);
    fclose(input);
    fclose(intermediate);
    return 0;
}