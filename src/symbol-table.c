#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbol-table.h"

#define INITIAL_SIZE 4
#define LOAD_FACTOR_THRESHOLD 0.7

// Uses the DJB2 hash function to hash the string.
static uint32_t hash(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; 
    return hash;
}

// Creates a table and returns a pointer to it.
SymbolTable *create_table() {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->table = calloc(INITIAL_SIZE, sizeof(Symbol *));
    table->len = 0;
    table->size = INITIAL_SIZE;
    return table;
}

// Frees all memory associated with `table`
void free_table(SymbolTable *table) {
  for (int i = 0; i < table->size; i++) {
    Symbol *entry = table->table[i];
    while (entry) {
      Symbol *temp = entry;
      entry = entry->next;
      free(temp->name);
      free(temp);
    }
  }
  free(table->table);
  free(table);
}

static int compare_symbols(const void *a, const void *b) {
    uint32_t addr_a = (*(Symbol **)a)->addr;
    uint32_t addr_b = (*(Symbol **)b)->addr;

    if (addr_a < addr_b) return -1;
    if (addr_a > addr_b) return 1;
    return 0;
}

// Prints out the table into the output stream.
void print_table(FILE *output, SymbolTable *table) {
    Symbol **symbols = malloc(table->len * sizeof(Symbol *));
    uint32_t count = 0;
    for (uint32_t i = 0; i < table->size; i++) {
        Symbol *entry = table->table[i];
        
        while (entry) {
            symbols[count++] = entry;
            entry = entry->next;
        }
    }
    qsort(symbols, table->len, sizeof(Symbol *), compare_symbols);
    for (uint32_t i = 0; i < table->len; i++) {
        fprintf(output, "%u\t\t\t%s\n", symbols[i]->addr, symbols[i]->name);
    }

    free(symbols);
}


// Resizes a table by a factor of two and rehashes table.
static void resize_table(SymbolTable *table) {
    uint32_t new_size = table->size * 2;
    Symbol **new_table = (Symbol **)calloc(new_size, sizeof(Symbol *));

    for (uint32_t i = 0; i < table->size; i++) {
        Symbol *entry = table->table[i];
        while (entry) {
            Symbol *next_entry = entry->next;
            
            uint32_t new_idx = hash(entry->name) % new_size;
            entry->next = new_table[new_idx];
            new_table[new_idx] = entry;
            
            entry = next_entry;
        }
    }

    free(table->table);
    table->table = new_table;
    table->size = new_size;
}

// Adds a symbol (name, addr) to `table`. If the size of the table is too small,
// the table will increase in length.
int add_to_table(SymbolTable *table, const char *name, uint32_t addr) {
    float load_factor = ((float)table->len) / table->size;
    if (load_factor > LOAD_FACTOR_THRESHOLD) {
        resize_table(table);
    }

    uint32_t idx = hash(name) % table->size;
    Symbol *new_entry = (Symbol *)malloc(sizeof(Symbol));
    new_entry->name = strdup(name);
    new_entry->addr = addr;
    new_entry->next = table->table[idx];
    table->table[idx] = new_entry;
    table->len++;

    return 0;
}

// Returns an address given an name entry.
int64_t get_from_table(SymbolTable *table, const char *name) {
    uint32_t idx = hash(name) % table->size;
    Symbol *entry = table->table[idx];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->addr;
        }
        entry = entry->next;
    }
    return -1;
}