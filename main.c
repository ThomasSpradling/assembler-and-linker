#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include "src/assembler.h"

// Takes in a file `filename` and returns whether or not a file exists.
static bool file_exists(char *filename) {
  struct stat buffer;
  return stat(filename, &buffer) == 0;
}

int main(int argc, char **argv) {
  if (argc < 2 || argc >= 4) {
    fprintf(stderr, "ERROR: Incorrect number of arguments. Usage: ./assembler <input_file> [output_file].\n");
    return 3;
  }
  int in_filename_length = strlen(argv[1]);
  if (!file_exists(argv[1]) || strcmp(argv[1] + in_filename_length - 2, ".s") != 0) {
    fprintf(stderr, "ERROR: Input file must exist and filename must end in `.s`.\n");
    return 3;
  }

  int out_filename_length = strlen(argv[1]);
  if (argc == 3 && !strcmp(argv[2] + out_filename_length - 4, ".out")) {
    fprintf(stderr, "ERROR: Output filename must end in `.out`.\n");
    return 3;
  }

  char *output_file = argc < 3 ? "a.out" : argv[2];
  int error_code = assemble(argv[1], output_file);
  if (error_code != 0) {
    fprintf(stderr, "One or more errors encountered during assembly operation.\n");
  }
  return error_code;
}