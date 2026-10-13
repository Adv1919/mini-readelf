// =============================================================================
// elf_types.h  —  Portable ELF type definitions
//
// Mirrors every type and constant you would normally get from <elf.h> on Linux.
// Defining them here makes the parser compile on any platform (Windows, macOS,
// Linux) without depending on a system header.
//
// Reference: System V ABI — ELF-64 Object File Format (v1.5)
//            Tool Interface Standard (TIS) Portable Formats Spec 1.1
// =============================================================================
#pragma once
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// Primitive ELF types
// ─────────────────────────────────────────────────────────────────────────────

using Elf32_Addr   = uint32_t;
using Elf32_Half   = uint16_t;
using Elf32_Off    = uint32_t;
using Elf32_Sword  = int32_t;
using Elf32_Word   = uint32_t;
using Elf32_Xword  = uint64_t;
using Elf32_Sxword = int64_t;

using Elf64_Addr   = uint64_t;
using Elf64_Half   = uint16_t;
using Elf64_Off    = uint64_t;
using Elf64_Sword  = int32_t;
using Elf64_Word   = uint32_t;
using Elf64_Xword  = uint64_t;
using Elf64_Sxword = int64_t;

// ─────────────────────────────────────────────────────────────────────────────
// ELF Identification (e_ident[] indices)
// ─────────────────────────────────────────────────────────────────────────────

constexpr int EI_NIDENT     = 16;  // Size of e_ident[]
constexpr int EI_MAG0       =  0;  // File identification
constexpr int EI_MAG1       =  1;
constexpr int EI_MAG2       =  2;
constexpr int EI_MAG3       =  3;
constexpr int EI_CLASS      =  4;  // File class (32/64)
constexpr int EI_DATA       =  5;  // Data encoding (endianness)
constexpr int EI_VERSION    =  6;  // File version
constexpr int EI_OSABI      =  7;  // OS / ABI identification
constexpr int EI_ABIVERSION =  8;  // ABI version

// Magic bytes
constexpr uint8_t ELFMAG0 = 0x7f;
constexpr uint8_t ELFMAG1 = 'E';
constexpr uint8_t ELFMAG2 = 'L';
constexpr uint8_t ELFMAG3 = 'F';

// EI_CLASS
constexpr uint8_t ELFCLASSNONE = 0;
constexpr uint8_t ELFCLASS32   = 1;  // 32-bit
constexpr uint8_t ELFCLASS64   = 2;  // 64-bit

// EI_DATA
constexpr uint8_t ELFDATANONE = 0;
constexpr uint8_t ELFDATA2LSB = 1;  // Little-endian
constexpr uint8_t ELFDATA2MSB = 2;  // Big-endian

// EI_VERSION / e_version
constexpr uint8_t EV_NONE    = 0;
constexpr uint8_t EV_CURRENT = 1;

// EI_OSABI
constexpr uint8_t ELFOSABI_NONE       =   0;  // UNIX System V
constexpr uint8_t ELFOSABI_HPUX       =   1;
constexpr uint8_t ELFOSABI_NETBSD     =   2;
constexpr uint8_t ELFOSABI_GNU        =   3;  // Also LINUX
constexpr uint8_t ELFOSABI_LINUX      =   3;
constexpr uint8_t ELFOSABI_SOLARIS    =   6;
constexpr uint8_t ELFOSABI_FREEBSD    =   9;
constexpr uint8_t ELFOSABI_OPENBSD    =  12;
constexpr uint8_t ELFOSABI_STANDALONE = 255;

// ─────────────────────────────────────────────────────────────────────────────
// e_type  —  Object file type
// ─────────────────────────────────────────────────────────────────────────────

constexpr uint16_t ET_NONE = 0;  // Unknown
constexpr uint16_t ET_REL  = 1;  // Relocatable object (.o)
constexpr uint16_t ET_EXEC = 2;  // Executable
constexpr uint16_t ET_DYN  = 3;  // Shared library / PIE executable
constexpr uint16_t ET_CORE = 4;  // Core dump

// ─────────────────────────────────────────────────────────────────────────────
// e_machine  —  Target architecture
// ─────────────────────────────────────────────────────────────────────────────

constexpr uint16_t EM_NONE    =   0;
constexpr uint16_t EM_386     =   3;  // Intel IA-32
constexpr uint16_t EM_MIPS    =   8;  // MIPS R3000
constexpr uint16_t EM_PPC     =  20;  // PowerPC 32-bit
constexpr uint16_t EM_PPC64   =  21;  // PowerPC 64-bit
constexpr uint16_t EM_S390    =  22;  // IBM S/390
constexpr uint16_t EM_ARM     =  40;  // ARM (32-bit)
constexpr uint16_t EM_X86_64  =  62;  // AMD64 / Intel 64
constexpr uint16_t EM_AARCH64 = 183;  // ARM 64-bit (AArch64)
constexpr uint16_t EM_CUDA    = 190;  // NVIDIA CUDA (PTX embedded in ELF)
constexpr uint16_t EM_RISCV   = 243;  // RISC-V

// ─────────────────────────────────────────────────────────────────────────────
// ELF Structures — use pragma pack to guarantee no compiler-inserted padding
// ─────────────────────────────────────────────────────────────────────────────

#pragma pack(push, 1)

// ── ELF Headers ─────────────────────────────────────────────────────────────

struct Elf32_Ehdr {
    uint8_t    e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off  e_phoff;       // program header table offset
    Elf32_Off  e_shoff;       // section header table offset
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;    // index of section-name string table
};

struct Elf64_Ehdr {
    uint8_t    e_ident[EI_NIDENT];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;
    Elf64_Off  e_phoff;
    Elf64_Off  e_shoff;
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
};

// ── Program Headers (Segments) ───────────────────────────────────────────────
// IMPORTANT: p_flags lives at a DIFFERENT offset in 32-bit vs 64-bit ELF.
//   Elf32_Phdr: ... p_filesz, p_memsz, p_flags, p_align      (flags at byte 24)
//   Elf64_Phdr: p_type, p_flags, p_offset, ...                (flags at byte 4)
// This is the most common source of bugs in ELF parsers.

struct Elf32_Phdr {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;   // ← at byte 24
    Elf32_Word p_align;
};

struct Elf64_Phdr {
    Elf64_Word  p_type;
    Elf64_Word  p_flags;  // ← at byte 4 (moved here for better alignment)
    Elf64_Off   p_offset;
    Elf64_Addr  p_vaddr;
    Elf64_Addr  p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
};

// ── Section Headers ──────────────────────────────────────────────────────────

struct Elf32_Shdr {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off  sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
};

struct Elf64_Shdr {
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word  sh_link;     // section-type-dependent: often index of assoc table
    Elf64_Word  sh_info;     // section-type-dependent: often count or index
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
};

// ── Symbol Table Entries ─────────────────────────────────────────────────────
// IMPORTANT: Field order differs between 32-bit and 64-bit.
//   Elf32_Sym: st_name, st_value, st_size, st_info, st_other, st_shndx
//   Elf64_Sym: st_name, st_info, st_other, st_shndx, st_value, st_size

struct Elf32_Sym {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    uint8_t    st_info;    // bind (high 4 bits) | type (low 4 bits)
    uint8_t    st_other;   // visibility
    Elf32_Half st_shndx;
};

struct Elf64_Sym {
    Elf64_Word  st_name;
    uint8_t     st_info;   // ← moved up for padding efficiency
    uint8_t     st_other;
    Elf64_Half  st_shndx;
    Elf64_Addr  st_value;
    Elf64_Xword st_size;
};

// ── Relocation Entries ───────────────────────────────────────────────────────

struct Elf32_Rel {
    Elf32_Addr r_offset;
    Elf32_Word r_info;     // sym (high 24 bits) | type (low 8 bits)
};

struct Elf64_Rel {
    Elf64_Addr  r_offset;
    Elf64_Xword r_info;    // sym (high 32 bits) | type (low 32 bits)
};

struct Elf32_Rela {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
    Elf32_Sword r_addend;
};

struct Elf64_Rela {
    Elf64_Addr   r_offset;
    Elf64_Xword  r_info;
    Elf64_Sxword r_addend;
};

// ── Dynamic Section Entries ──────────────────────────────────────────────────

struct Elf32_Dyn {
    Elf32_Sword d_tag;
    union { Elf32_Word d_val; Elf32_Addr d_ptr; } d_un;
};

struct Elf64_Dyn {
    Elf64_Sxword d_tag;
    union { Elf64_Xword d_val; Elf64_Addr d_ptr; } d_un;
};

#pragma pack(pop)

// ─────────────────────────────────────────────────────────────────────────────
// r_info field decomposition macros
// ─────────────────────────────────────────────────────────────────────────────

#define ELF32_R_SYM(i)    (static_cast<uint32_t>(i) >> 8)
#define ELF32_R_TYPE(i)   (static_cast<uint8_t>(i))
#define ELF64_R_SYM(i)    (static_cast<uint64_t>(i) >> 32)
#define ELF64_R_TYPE(i)   (static_cast<uint32_t>((i) & 0xffffffffULL))

// st_info field decomposition macros
#define ELF32_ST_BIND(i)  ((static_cast<uint8_t>(i)) >> 4)
#define ELF32_ST_TYPE(i)  ((static_cast<uint8_t>(i)) & 0xf)
#define ELF64_ST_BIND(i)  ELF32_ST_BIND(i)
#define ELF64_ST_TYPE(i)  ELF32_ST_TYPE(i)

// ─────────────────────────────────────────────────────────────────────────────
// Segment types (p_type)
// ─────────────────────────────────────────────────────────────────────────────

constexpr uint32_t PT_NULL         = 0;
constexpr uint32_t PT_LOAD         = 1;   // Loadable segment
constexpr uint32_t PT_DYNAMIC      = 2;   // Dynamic linking info
constexpr uint32_t PT_INTERP       = 3;   // Dynamic linker path
constexpr uint32_t PT_NOTE         = 4;   // Auxiliary information
constexpr uint32_t PT_SHLIB        = 5;
constexpr uint32_t PT_PHDR         = 6;   // Program header table itself
constexpr uint32_t PT_TLS          = 7;   // Thread-local storage
constexpr uint32_t PT_GNU_EH_FRAME = 0x6474e550;
constexpr uint32_t PT_GNU_STACK    = 0x6474e551;
constexpr uint32_t PT_GNU_RELRO    = 0x6474e552;
constexpr uint32_t PT_GNU_PROPERTY = 0x6474e553;

// Segment flags (p_flags)
constexpr uint32_t PF_X = 0x1;  // Execute
constexpr uint32_t PF_W = 0x2;  // Write
constexpr uint32_t PF_R = 0x4;  // Read

// ─────────────────────────────────────────────────────────────────────────────
// Section types (sh_type)
// ─────────────────────────────────────────────────────────────────────────────

constexpr uint32_t SHT_NULL          =  0;
constexpr uint32_t SHT_PROGBITS      =  1;  // Program data
constexpr uint32_t SHT_SYMTAB        =  2;  // Full symbol table
constexpr uint32_t SHT_STRTAB        =  3;  // String table
constexpr uint32_t SHT_RELA          =  4;  // Relocation entries with addends
constexpr uint32_t SHT_HASH          =  5;  // Symbol hash table
constexpr uint32_t SHT_DYNAMIC       =  6;  // Dynamic linking info
constexpr uint32_t SHT_NOTE          =  7;  // Notes / build-id
constexpr uint32_t SHT_NOBITS        =  8;  // No-data section (.bss)
constexpr uint32_t SHT_REL           =  9;  // Relocation entries without addends
constexpr uint32_t SHT_SHLIB         = 10;
constexpr uint32_t SHT_DYNSYM        = 11;  // Dynamic symbol table
constexpr uint32_t SHT_INIT_ARRAY    = 14;
constexpr uint32_t SHT_FINI_ARRAY    = 15;
constexpr uint32_t SHT_PREINIT_ARRAY = 16;
constexpr uint32_t SHT_GNU_HASH      = 0x6ffffff6;
constexpr uint32_t SHT_GNU_VERSYM    = 0x6fffffff;
constexpr uint32_t SHT_GNU_VERNEED   = 0x6ffffffe;

// Section flags (sh_flags)
constexpr uint64_t SHF_WRITE      = 0x001;  // Writable
constexpr uint64_t SHF_ALLOC      = 0x002;  // Occupies memory during execution
constexpr uint64_t SHF_EXECINSTR  = 0x004;  // Executable code
constexpr uint64_t SHF_MERGE      = 0x010;  // Mergeable
constexpr uint64_t SHF_STRINGS    = 0x020;  // Contains null-terminated strings
constexpr uint64_t SHF_INFO_LINK  = 0x040;  // sh_info holds section index
constexpr uint64_t SHF_LINK_ORDER = 0x080;  // Preserve order after combining
constexpr uint64_t SHF_TLS        = 0x400;  // Thread-local storage

// Special section index values
constexpr uint16_t SHN_UNDEF  = 0;       // Undefined
constexpr uint16_t SHN_ABS    = 0xfff1;  // Absolute value (not relocated)
constexpr uint16_t SHN_COMMON = 0xfff2;  // Common block symbol

// ─────────────────────────────────────────────────────────────────────────────
// Symbol binding (high nibble of st_info)
// ─────────────────────────────────────────────────────────────────────────────

constexpr uint8_t STB_LOCAL  = 0;  // Not visible outside the object
constexpr uint8_t STB_GLOBAL = 1;  // Visible to all combined objects
constexpr uint8_t STB_WEAK   = 2;  // Like global but lower precedence

// Symbol type (low nibble of st_info)
constexpr uint8_t STT_NOTYPE  = 0;  // Not specified
constexpr uint8_t STT_OBJECT  = 1;  // Data object (variable, array, …)
constexpr uint8_t STT_FUNC    = 2;  // Function / executable code
constexpr uint8_t STT_SECTION = 3;  // Associated with a section
constexpr uint8_t STT_FILE    = 4;  // Source file name
constexpr uint8_t STT_COMMON  = 5;  // Uninitialized common block
constexpr uint8_t STT_TLS     = 6;  // Thread-local data object

// Symbol visibility (st_other & 0x3)
constexpr uint8_t STV_DEFAULT   = 0;
constexpr uint8_t STV_INTERNAL  = 1;
constexpr uint8_t STV_HIDDEN    = 2;
constexpr uint8_t STV_PROTECTED = 3;

// ─────────────────────────────────────────────────────────────────────────────
// Dynamic tags (d_tag)
// ─────────────────────────────────────────────────────────────────────────────

constexpr int64_t DT_NULL       =  0;
constexpr int64_t DT_NEEDED     =  1;   // Name of needed library (d_val = .dynstr offset)
constexpr int64_t DT_PLTRELSZ   =  2;
constexpr int64_t DT_PLTGOT     =  3;
constexpr int64_t DT_HASH       =  4;
constexpr int64_t DT_STRTAB     =  5;
constexpr int64_t DT_SYMTAB     =  6;
constexpr int64_t DT_RELA       =  7;
constexpr int64_t DT_RELASZ     =  8;
constexpr int64_t DT_RELAENT    =  9;
constexpr int64_t DT_STRSZ      = 10;
constexpr int64_t DT_SYMENT     = 11;
constexpr int64_t DT_INIT       = 12;
constexpr int64_t DT_FINI       = 13;
constexpr int64_t DT_SONAME     = 14;
constexpr int64_t DT_RPATH      = 15;
constexpr int64_t DT_SYMBOLIC   = 16;
constexpr int64_t DT_REL        = 17;
constexpr int64_t DT_RELSZ      = 18;
constexpr int64_t DT_RELENT     = 19;
constexpr int64_t DT_PLTREL     = 20;
constexpr int64_t DT_DEBUG      = 21;
constexpr int64_t DT_TEXTREL    = 22;
constexpr int64_t DT_JMPREL     = 23;
constexpr int64_t DT_BIND_NOW   = 24;
constexpr int64_t DT_FLAGS      = 30;
constexpr int64_t DT_GNU_HASH   = 0x6ffffef5LL;
constexpr int64_t DT_VERSYM     = 0x6ffffff0LL;
constexpr int64_t DT_FLAGS_1    = 0x6ffffffbLL;
constexpr int64_t DT_VERNEED    = 0x6ffffffeLL;
constexpr int64_t DT_VERNEEDNUM = 0x6fffffffLL;

// ─────────────────────────────────────────────────────────────────────────────
// x86-64 relocation types (r_type field from r_info)
// ─────────────────────────────────────────────────────────────────────────────

constexpr uint32_t R_X86_64_NONE          =  0;
constexpr uint32_t R_X86_64_64            =  1;  // Direct 64-bit
constexpr uint32_t R_X86_64_PC32          =  2;  // PC-relative 32-bit
constexpr uint32_t R_X86_64_GOT32         =  3;  // 32-bit GOT entry
constexpr uint32_t R_X86_64_PLT32         =  4;  // 32-bit PLT address
constexpr uint32_t R_X86_64_COPY          =  5;  // Copy symbol at runtime
constexpr uint32_t R_X86_64_GLOB_DAT      =  6;  // Create GOT entry
constexpr uint32_t R_X86_64_JUMP_SLOT     =  7;  // Create PLT entry
constexpr uint32_t R_X86_64_RELATIVE      =  8;  // Adjust by program base
constexpr uint32_t R_X86_64_GOTPCREL      =  9;
constexpr uint32_t R_X86_64_32            = 10;
constexpr uint32_t R_X86_64_32S           = 11;
constexpr uint32_t R_X86_64_PC64          = 24;
constexpr uint32_t R_X86_64_GOTPC32       = 26;
constexpr uint32_t R_X86_64_REX_GOTPCRELX = 42;

// ─────────────────────────────────────────────────────────────────────────────
// ARM (32-bit) relocation types
// ─────────────────────────────────────────────────────────────────────────────

constexpr uint32_t R_ARM_NONE      =  0;
constexpr uint32_t R_ARM_PC24      =  1;
constexpr uint32_t R_ARM_ABS32     =  2;
constexpr uint32_t R_ARM_CALL      = 28;
constexpr uint32_t R_ARM_JUMP24    = 29;
constexpr uint32_t R_ARM_GLOB_DAT  = 21;
constexpr uint32_t R_ARM_JUMP_SLOT = 22;
constexpr uint32_t R_ARM_RELATIVE  = 23;
constexpr uint32_t R_ARM_COPY      = 20;
