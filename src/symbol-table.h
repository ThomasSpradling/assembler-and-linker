#include <stdint.h>

typedef struct Symbol {
    char *name;
    uint32_t addr;
    struct Symbol *next;
} Symbol;

typedef struct {
    Symbol **table;
    uint32_t len;
    uint32_t size;
} SymbolTable;

SymbolTable *create_table();
void free_table(SymbolTable *table);
int add_to_table(SymbolTable *table, const char *name, uint32_t addr);
int64_t get_from_table(SymbolTable *table, const char *name);
void print_table(FILE *output, SymbolTable *table);