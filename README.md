# Assembler and Linker

This project largely is to help gain a more full appreciation of the MIPS architecture and its assembly lanuage. Here, I have created a two-pass assembler with an associated linker.

## Assembler
The assembler will automatically create a symbol table and relocation table, so that the linker can eventually link any unresolved labels (i.e. those caused by jump instrunctions).

To use the assembler, run
```bash
make assembler
```

This will create a binary file that can be ran to assemble some code using:
```bash
./bin/assembler <input_file> [output_file]
```

If no output is provided, it will be placed in `a.out`.

## Linker
TODO

