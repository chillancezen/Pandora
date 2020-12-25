#ifndef _SYMBOL_H
#define _SYMBOL_H
#include <stdio.h>

enum symbol_type {
    SYMBOL_TYPE_GLOBAL_VARIABLE = 0x01,
    SYMBOL_TYPE_FUNCTION = 0x02,
    SYMBOL_TYPE_PARAMTER = 0x04,
    SYMBOL_TYPE_LOCAL_VARIABLE = 0x08
};
struct symbol {
    // a reference to name string. Never try to free it!
    char * symbol_name;
    enum symbol_type type;
    void * blob;

    int visibility_level;
    struct symbol * next_symbol;
    struct symbol * prev_symbol;
};


struct symbol *
new_symbol(enum symbol_type type,
           char * symbol_name,
           void * blob,
           int visibility_level);

void
append_symbol(struct symbol ** symbol_head,
              struct symbol * target);

struct symbol *
pop_symbol(struct symbol ** symbol_head);

void
free_symbol(struct symbol * sym);

int
is_symbol_present(struct symbol ** symbols,
                  char * symbol_name,
                  enum symbol_type type);

struct symbol *
lookup_symbol(struct symbol ** symbols,
              char * symbol_name,
              enum symbol_type type);

int
is_symbol_present_at_visibility_level(struct symbol ** symbols,
                                    char * symbol_name,
                                    enum symbol_type type,
                                    int visibility_level);

#endif
