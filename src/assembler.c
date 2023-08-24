#include "assembler.h"

// Strips away all comments and adds all labels to a symbol table, and writes these changes into a temporary
// `.int` file.
static int clean_file(char *input_file, SymbolTable *table) {
    // setup
    FILE *input = fopen(input_file, "r");
    if (input == NULL) {
        fprintf(stderr, "ERROR: Failed to open file %s.\n", input_file);
        return 3;
    }

    FILE *intermediate = fopen("temp.int", "w");
    if (intermediate == NULL) {
        fclose(input);
        fprintf(stderr, "ERROR: Failed to create intermediate file `temp.int`.\n");
        return 3;
    }

    int line_number = 0;
    int byte_offset = 0;
    char line[100];
    int is_error = 0;

    while (fgets(line, sizeof(line), input) != NULL) {
        line_number++;

        // strip comment by replacing it with `\0` character
        char *comment = strchr(line, '#');
        if (comment != NULL) {
            *comment = '\0';
        }

        // adds label into symbol table and sets clips off label
        char *label_end = strchr(line, ':');
        if (label_end != NULL) {
            *label_end = '\0';
            if (!is_valid_label(line)) {
                fprintf(stderr, "ERROR: Invalid label at line %i: %s.\n", line_number, line);
                is_error = 1;
                continue;
            }
            if (get_from_table(table, line) != -1) {
                fprintf(stderr, "ERROR: Name '%s' label at line %i already defined.\n", line, line_number);
                is_error = 1;
                continue;
            }
            add_to_table(table, line, byte_offset);
            memmove(line, label_end + 1, strlen(label_end + 1) + 1);
        }

        // finds op name as first non-whitespace token
        char *name = strtok(line, " \t\n");
        if (name == NULL) {
            continue;
        }
        
        // grabs each argument, ignoring whitespace
        char *args[4];
        int num_args = 0;
        char *current;
        while ((current = strtok(NULL, ", \t\n()")) != NULL && num_args < 4) {
            args[num_args] = current;
            num_args++;
        }

        if (num_args > 3) {
            fprintf(stderr, "ERROR: Extra argument at line %i: %s.\n", line_number, args[3]);
            is_error = 1;
            continue;
        }

        byte_offset += 4 * write_instruction(intermediate, name, args, num_args);
    }

    // cleanup
    fclose(input);
    fclose(intermediate);

    return is_error;
}

int translate_file(char *input_file, char *output_file, SymbolTable *table, SymbolTable *relocation_table) {
    // Setup
    FILE *input = fopen(input_file, "r");
    if (input == NULL) {
        fprintf(stderr, "ERROR: Failed to open file %s.\n", input_file);
        return 3;
    }

    FILE *output = fopen(output_file, "w");
    if (output == NULL) {
        fclose(input);
        fprintf(stderr, "ERROR: Failed to open file %s.\n", output_file);
        return 3;
    }

    fprintf(output, ".text\n");

    int err_code = 0;
    int line_num = 0;
    int byte_offset = 0;

    char line[100];
    while (fgets(line, sizeof(line), input) != NULL) {
        line_num++;

        char current_line[100];
        strcpy(current_line, line);

        char *current = strtok(line, " \n");
        char *name = current;

        char *args[3];
        int num_args = 0;
        while ((current = strtok(NULL, " \n")) != NULL && num_args < 3) {
            args[num_args] = current;
            num_args++;
        }

        err_code = translate_instruction(output, name, args, num_args, table, relocation_table, byte_offset);
        if (err_code != 0) {
            fprintf(stderr, "ERROR: Invalid instruction at line %i: %s", line_num, current_line);
        }
        byte_offset += 4;
    }

    fprintf(output, "\n.symbol\n");
    print_table(output, table);

    fprintf(output, "\n.relocation\n");
    print_table(output, relocation_table);

    return err_code;
}

// Reads filename `f` and assembles it in two-passes.
//  (1)   The first pass will strip any comments and generate a symbol table
// all into an intermediate .int file. After this pass is complete, the file `f` will
// be closed.
//  (2)   The second pass will read this .int file and produce the .out file
// from this cleaned up assembly file.
//
//  -     Afterward, the .int file will be removed.
int assemble(char *input_file, char *output_file) {
    SymbolTable *table = create_table();
    int clean_exit_code = clean_file(input_file, table);

    if (clean_exit_code != 0) {
        free(table);
        return clean_exit_code;
    }


    SymbolTable *relocation_table = create_table();
    int translate_exit_code = translate_file("temp.int", output_file, table, relocation_table);
    // DELETE FILE

    free(table);
    free(relocation_table);
    return translate_exit_code;
}