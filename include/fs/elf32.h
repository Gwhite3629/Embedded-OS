#ifndef _ELF32_H_
#define _ELF32_H_

#include "../stdlib.h"

typedef uint32_t elf32_word;
typedef uint32_t elf32_addr;
typedef uint16_t elf32_half;
typedef uint32_t elf32_off;
typedef int32_t  elf32_Sword;

#define E_NIDENT 16

typedef struct {
    unsigned char   e_ident[E_NIDENT];  // ELF identification information
    elf32_half      e_type;             // Object file type
    elf32_half      e_machine;          // Architecture
    elf32_word      e_version;          // Object file version
    elf32_addr      e_entry;            // Virtual address holding program
                                        // entry
    elf32_off       e_phoff;            // Programm header offset
    elf32_off       e_shoff;            // Section header offset
    elf32_word      e_flags;            // Processor flags
    elf32_half      e_ehsize;           // ELF header size
    elf32_half      e_phentsize;        // Size of program header entries
    elf32_half      e_phnum;            // Number of program header entries
    elf32_half      e_shentsize;        // Size of section header
    elf32_half      e_shnum;            // Number of section header entries
    elf32_half      e_shstrndx;         // Section header table index which
                                        // holds the section name string table
} elf32_header;

typedef struct {
    elf32_word      sh_name;            // Section name, index into string
                                        // table
    elf32_word      sh_type;            // Section type
    elf32_word      sh_flags;           // Section attributes
    elf32_addr      sh_addr;            // Sections memory address, will be
                                        // 0 if section isn't in the processes
                                        //   memory map
    elf32_off       sh_offset;          // File offset of section
    elf32_word      sh_size;            // Section size
    elf32_word      sh_link;            // Section header table index link
    elf32_word      sh_info;            // Extra information, based on type
    elf32_word      sh_addralign;       // Address alignment, sh_addr must be
                                        // congruent to this value
    elf32_word      sh_entrysize;       // Section symbol table entry size
} elf32_shdr;

typedef struct {
    elf32_word      st_name;            // Symbol name, index into string table
    elf32_addr      st_value;           // Value of symbol
    elf32_word      st_size;            // Size of symbol in bytes
    unsigned char   st_info;            // Symbol types and attributes
    unsigned char   st_other;           // Undefined
    elf32_half      st_shndx;           // Section header table index
} elf32_sym;

typedef struct {
    elf32_addr      r_offset;           // Location to apply relocation
    elf32_word      r_info;             // Type of relocation and section table
                                        // index to which the relocation is made
} elf32_rel;

typedef struct {
    elf32_addr      r_offset;           // Location to apply relocation
    elf32_word      r_info;             // Type of relocation and section table
                                        // index to which the relocation is made
    elf32_Sword     r_addend;           // Constant value used to calculate the
                                        // value to be stored in the relocation
                                        // field
} elf32_rela;

typedef struct {
    elf32_word      p_type;             // Type of segment
    elf32_off       p_offset;           // File offset for beginning of segment
    elf32_addr      p_vaddr;            // Virtual address of segment in memory
    elf32_addr      p_paddr;            // Physical address of segment, may not
                                        // be valid
    elf32_word      p_filesz;           // Size of segment in file in bytes
    elf32_word      p_memsz;            // Size of segment in memory in bytes
    elf32_word      p_flags;            // Segment flags
    elf32_word      p_align;            // Segment address alignment, must be
                                        // congruent to 0 mod page size
} elf32_phdr;

#endif // _ELF32_H_
