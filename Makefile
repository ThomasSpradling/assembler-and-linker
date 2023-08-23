assembler: main.c src/assembler.c src/symbol-table.c src/read.c
	gcc -std=c99 main.c src/assembler.c src/symbol-table.c src/read.c -o bin/assembler

clean:
	rm bin/assembler