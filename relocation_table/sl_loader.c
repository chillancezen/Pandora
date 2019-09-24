#include <stdio.h>
#include <sys/mman.h>
#include <elf.h>
#include <assert.h>
#include <alloca.h>
#include <stdlib.h>
#include <string.h>

static int
load_file_into_memory(FILE * fp, int offset, int length, void * buff)
{
    fseek(fp, offset, SEEK_SET);
    return fread(buff, 1, length, fp);
}

static void
get_sec_string(FILE * fp, int offset, char * name)
{
    Elf64_Ehdr *elf_hdr = alloca(sizeof(Elf64_Ehdr));
    load_file_into_memory(fp, 0, sizeof(Elf64_Ehdr), elf_hdr);
    int string_section_index = elf_hdr->e_shstrndx;
    Elf64_Shdr * string_hdr = alloca(sizeof(Elf64_Shdr));
    load_file_into_memory(fp, sizeof(Elf64_Shdr) * string_section_index + elf_hdr->e_shoff,
                          sizeof(Elf64_Shdr), string_hdr);

    char * all_strings = alloca(string_hdr->sh_size);
    load_file_into_memory(fp, string_hdr->sh_offset, string_hdr->sh_size, all_strings);
    assert(offset < string_hdr->sh_size);
    strcpy(name, all_strings + offset);
}

static void
dump_sections(FILE * fp)
{
    Elf64_Ehdr *elf_hdr = alloca(sizeof(Elf64_Ehdr));
    load_file_into_memory(fp, 0, sizeof(Elf64_Ehdr), elf_hdr);

    void * sections = alloca(elf_hdr->e_shnum * sizeof(Elf64_Shdr));
    load_file_into_memory(fp, elf_hdr->e_shoff, elf_hdr->e_shnum * sizeof(Elf64_Shdr), sections);
    int index;
    char sec_name[64];
    for (index = 0; index < elf_hdr->e_shnum; index++) {
        Elf64_Shdr * sec = (Elf64_Shdr *)(sections + index * sizeof(Elf64_Shdr));
        memset(sec_name, 0x0, sizeof(sec_name));
        get_sec_string(fp, sec->sh_name, sec_name);
        printf("%s\n", sec_name);
    }
}

static int
get_section_header(FILE * fp, const char * section_name, void * section)
{
    Elf64_Ehdr *elf_hdr = alloca(sizeof(Elf64_Ehdr));
    load_file_into_memory(fp, 0, sizeof(Elf64_Ehdr), elf_hdr);
    void * sections = alloca(elf_hdr->e_shnum * sizeof(Elf64_Shdr));
    load_file_into_memory(fp, elf_hdr->e_shoff, elf_hdr->e_shnum * sizeof(Elf64_Shdr), sections);

    int found = 0;
    int index;
    char sec_name[64];
    for (index = 0; index < elf_hdr->e_shnum; index++) {
        Elf64_Shdr * sec = (Elf64_Shdr *)(sections + index * sizeof(Elf64_Shdr));
        memset(sec_name, 0x0, sizeof(sec_name));
        get_sec_string(fp, sec->sh_name, sec_name);
        if (!strcmp(sec_name, section_name)) {
            found = 1;
            memcpy(section, sec, sizeof(Elf64_Shdr));
        }
    }
    return found;
}


static void
get_symbol_string(FILE * fp, int offset, char * name)
{
    Elf64_Shdr string_hdr;
    assert(get_section_header(fp, ".strtab", &string_hdr));

    char * all_strings = alloca(string_hdr.sh_size);
    load_file_into_memory(fp, string_hdr.sh_offset, string_hdr.sh_size, all_strings);
    assert(offset < string_hdr.sh_size);
    strcpy(name, all_strings + offset);
}

static void
dump_symbols(FILE * fp)
{
    Elf64_Shdr sym_hdr;
    assert(get_section_header(fp, ".symtab", &sym_hdr));
    Elf64_Sym * all_syms = alloca(sym_hdr.sh_size);
    load_file_into_memory(fp, sym_hdr.sh_offset, sym_hdr.sh_size, all_syms);

    char sym_name[64];
    Elf64_Sym * sym = all_syms;
    for (; (uint64_t)sym < (sym_hdr.sh_size + (uint64_t)(all_syms)); sym++) {
        memset(sym_name, 0x0, sizeof(sym_name));
        get_symbol_string(fp, sym->st_name, sym_name);
        printf("%s %x\n", sym_name, sym->st_value);
    }
}


static int
get_symbol(FILE * fp, const char * sym_name, Elf64_Sym * sym)
{
    int found = 0;
    Elf64_Shdr sym_hdr;
    assert(get_section_header(fp, ".symtab", &sym_hdr));
    Elf64_Sym * all_syms = alloca(sym_hdr.sh_size);
    load_file_into_memory(fp, sym_hdr.sh_offset, sym_hdr.sh_size, all_syms);
    char _sym_name[64];
    Elf64_Sym * _sym = all_syms;
    for (; (uint64_t)_sym < (sym_hdr.sh_size + (uint64_t)(all_syms)); _sym++) {
        memset(_sym_name, 0x0, sizeof(_sym_name));
        get_symbol_string(fp, _sym->st_name, _sym_name);
        if (!strcmp(sym_name, _sym_name)) {
            memcpy(sym, _sym, sizeof(Elf64_Sym));
            found = 1;
        }
    }
    return found;
}

static void
get_dynamic_symbol_string(FILE * fp, int offset, char * name)
{
    Elf64_Shdr string_hdr;
    assert(get_section_header(fp, ".dynstr", &string_hdr));

    char * all_strings = alloca(string_hdr.sh_size);
    load_file_into_memory(fp, string_hdr.sh_offset, string_hdr.sh_size, all_strings);
    assert(offset < string_hdr.sh_size);
    strcpy(name, all_strings + offset);
}


static int
get_dynamic_symbol(FILE * fp, const char * sym_name, Elf64_Sym * sym)
{
    int found = 0;
    Elf64_Shdr sym_hdr;
    assert(get_section_header(fp, ".dynsym", &sym_hdr));
    Elf64_Sym * all_syms = alloca(sym_hdr.sh_size);
    load_file_into_memory(fp, sym_hdr.sh_offset, sym_hdr.sh_size, all_syms);
    char _sym_name[64];
    Elf64_Sym * _sym = all_syms;
    for (; (uint64_t)_sym < (sym_hdr.sh_size + (uint64_t)(all_syms)); _sym++) {
        memset(_sym_name, 0x0, sizeof(_sym_name));
        get_dynamic_symbol_string(fp, _sym->st_name, _sym_name);
        if (!strcmp(sym_name, _sym_name)) {
            memcpy(sym, _sym, sizeof(Elf64_Sym));
            found = 1;
        }
    }
    return found;
}


static void
get_dynamic_symbol_by_index(FILE * fp, int index, Elf64_Sym * sym)
{
    Elf64_Shdr sym_hdr;
    assert(get_section_header(fp, ".dynsym", &sym_hdr));
    Elf64_Sym * all_syms = alloca(sym_hdr.sh_size);
    load_file_into_memory(fp, sym_hdr.sh_offset, sym_hdr.sh_size, all_syms);
    char _sym_name[64];
    assert(((uint64_t)(all_syms + index)) < (((uint64_t)(all_syms)) + sym_hdr.sh_size));
    memcpy(sym, all_syms + index, sizeof(Elf64_Sym));
}

static uint64_t
search_symbol(FILE * fp, uint64_t sl_base, const char * sym_name)
{
    Elf64_Sym sym;
    assert(get_symbol(fp, sym_name, &sym));
    return sym.st_value + sl_base;
}
static void
load_shared_library(const char * sl_path)
{
    FILE * fp = fopen(sl_path, "rb");
    assert(fp);
    Elf64_Ehdr *elf_hdr = alloca(sizeof(Elf64_Ehdr));
    assert(elf_hdr);
    assert(sizeof(Elf64_Ehdr) ==
           load_file_into_memory(fp, 0, sizeof(Elf64_Ehdr), elf_hdr));
    assert(elf_hdr->e_type == ET_DYN);
    Elf64_Phdr * program_hdrs = alloca(elf_hdr->e_phnum * sizeof(Elf64_Phdr));
    assert(program_hdrs);

    assert(load_file_into_memory(fp, elf_hdr->e_phoff,
                                 elf_hdr->e_phnum * sizeof(Elf64_Phdr), program_hdrs) ==
           elf_hdr->e_phnum * sizeof(Elf64_Phdr));
    
    int index = 0;
    int total_memory = 0x0;
    int align = 0;
    for (index = 0; index < elf_hdr->e_phnum; index++) {
        if (program_hdrs[index].p_type != PT_LOAD) {
            continue;
        }
        int ph_boundary = program_hdrs[index].p_vaddr + program_hdrs[index].p_memsz;
        if (ph_boundary & (program_hdrs[index].p_align - 1)) {
            ph_boundary &= ~(program_hdrs[index].p_align - 1);
            ph_boundary += program_hdrs[index].p_align;
        }
        if (total_memory < ph_boundary) {
            total_memory = ph_boundary;
        }
        if (align < program_hdrs[index].p_align) {
            align = program_hdrs[index].p_align;
        }
    }
    // for simplicity, we use malloc library to get a trunk of memory
    uint64_t sl_base = (uint64_t)malloc(total_memory + align);
    //uint64_t sl_base = (uint64_t)mmap(NULL, total_memory + align, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    sl_base &= ~(align - 1);
    sl_base += align;
    for (index = 0; index < elf_hdr->e_phnum; index++) {
        if (program_hdrs[index].p_type != PT_LOAD) {
            continue;
        }
        int nr_loaded =
            load_file_into_memory(fp, program_hdrs[index].p_offset, program_hdrs[index].p_filesz,
                                  (void *)(program_hdrs[index].p_vaddr + sl_base));
        assert(nr_loaded == program_hdrs[index].p_filesz);
    }
    // grant the permission
    for (index = 0; index < elf_hdr->e_phnum; index++) {
        if (program_hdrs[index].p_type != PT_LOAD) {
            continue;
        }
        int target_prot = PROT_NONE;
        if (program_hdrs[index].p_flags & PF_X) {
            target_prot |= PROT_EXEC;
        }
        if (program_hdrs[index].p_flags & PF_R) {
            target_prot |= PROT_READ;
        }
        if (program_hdrs[index].p_flags & PF_W) {
            target_prot |= PROT_WRITE;
        }

        uint64_t va_base = program_hdrs[index].p_vaddr;
        uint64_t va_size = program_hdrs[index].p_memsz;
        uint64_t alignment = program_hdrs[index].p_align;
        if (va_base & (alignment - 1)) {
            va_base &= ~(alignment - 1);
        }

        if (va_size & (alignment - 1)) {
            va_size &= ~(alignment - 1);
            va_size += alignment;
        }
        assert(!mprotect((void *)(sl_base + va_base), va_size, target_prot));
    }
    // patch dynamic relocation table
    {
        Elf64_Shdr rela_hdr;
        assert(get_section_header(fp, ".rela.dyn", &rela_hdr));
        Elf64_Rela * all_relas = alloca(rela_hdr.sh_size);
        load_file_into_memory(fp, rela_hdr.sh_offset, rela_hdr.sh_size, all_relas);
        Elf64_Rela * rela = all_relas;

        Elf64_Sym sym;
        char dyn_sym_name[64];
        for (; ((uint64_t)rela) < (((uint64_t)all_relas) + rela_hdr.sh_size); rela++) {
            memset(dyn_sym_name, 0x0, sizeof(dyn_sym_name));
            get_dynamic_symbol_by_index(fp, ELF64_R_SYM(rela->r_info), &sym);
            get_dynamic_symbol_string(fp, sym.st_name, dyn_sym_name);
           
            uint64_t addr = sl_base + rela->r_offset;
            uint64_t symbol = sl_base + sym.st_value;
            *(uint64_t *)addr = symbol;
            printf("patching 0x%x symbol:%s value:0x%x addend:0x%x\n", rela->r_offset, dyn_sym_name,
                   sym.st_value, rela->r_addend);
        }

    }
    // TEST of symbol addressing
    {
#if 1
        printf("shared library base:0x%x\n", sl_base);
        int (*fun)(void) = (int (*)(void))search_symbol(fp, sl_base, "foo");
        int * baz_ptr = (int *)search_symbol(fp, sl_base, "baz");
        *baz_ptr += 1000; 
        printf("fun called in shared library:%d\n", fun());
#endif
    }
}

int
main(void)
{
    load_shared_library("./libfoo.so");
    return 0;
}
