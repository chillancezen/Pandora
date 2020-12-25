#include <symbol.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct symbol *
new_symbol(enum symbol_type type,
           char * symbol_name,
           void * blob,
           int visibility_level)
{
    struct symbol * sym = (struct symbol *)malloc(sizeof(struct symbol));
    assert(sym);
    memset(sym, 0x0, sizeof(struct symbol));

    sym->type = type;
    sym->symbol_name = symbol_name;
    sym->blob = blob;
    sym->visibility_level = visibility_level;
    return sym;
}

void
append_symbol(struct symbol ** symbol_head,
              struct symbol * target)
{
    assert(!target->next_symbol);
    assert(!target->prev_symbol);
    if (!*symbol_head) {
        *symbol_head = target;
    } else {
        struct symbol * ptr = *symbol_head;
        for (; ptr->next_symbol; ptr = ptr->next_symbol);
        ptr->next_symbol = target;
        target->prev_symbol = ptr;
    }
}

struct symbol *
pop_symbol(struct symbol ** symbol_head)
{
    struct symbol * ptr = *symbol_head;
    if (!ptr) {
        return NULL;
    }

    // remove last symbol
    for (; ptr->next_symbol; ptr = ptr->next_symbol);

    if (!ptr->prev_symbol) {
        assert(*symbol_head == ptr);
        *symbol_head = NULL;
    } else {
        assert(*symbol_head != ptr);
        ptr->prev_symbol->next_symbol = NULL;
        ptr->prev_symbol = NULL;
    }

    return ptr;
}


void
free_symbol(struct symbol * sym)
{
    assert(!sym->prev_symbol);
    assert(!sym->next_symbol);
    free(sym);
}


int
is_symbol_present(struct symbol ** symbols,
                  char * symbol_name,
                  enum symbol_type type)
{
    struct symbol * ptr = *symbols;
    for (; ptr; ptr = ptr->next_symbol) {
        if (!strcmp(ptr->symbol_name, symbol_name) &&
            ptr->type & type) {
            return 1;
        }
    }
    return 0;
}

struct symbol *
lookup_symbol(struct symbol ** symbols,
              char * symbol_name,
              enum symbol_type type)
{
    struct symbol * ptr = *symbols;
    for (; ptr; ptr = ptr->next_symbol) {
        if (!strcmp(ptr->symbol_name, symbol_name) &&
            ptr->type & type) {
            return ptr;
        }
    }
    return NULL;
}

int
is_symbol_present_at_visibility_level(struct symbol ** symbols,
                                    char * symbol_name,
                                    enum symbol_type type,
                                    int visibility_level)
{
    struct symbol * ptr = *symbols;
    for (; ptr; ptr = ptr->next_symbol) {
        if (!strcmp(ptr->symbol_name, symbol_name) &&
            ptr->type & type &&
            ptr->visibility_level == visibility_level) {
            return 1;
        }
    }
    return 0;
}

