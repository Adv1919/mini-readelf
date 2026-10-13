import struct
import os

# Generates a valid minimal ELF-64 executable binary for x86-64 testing
# Layout:
# 0x0000: Elf64_Ehdr (64 bytes)
# 0x0040: Elf64_Phdr (56 bytes) [1 PT_LOAD segment]
# 0x0078: Code (.text section: 4 bytes: nop; ret)
# 0x0080: String table (.shstrtab: "\0.text\0.shstrtab\0.symtab\0.strtab\0")
# 0x0100: Symbol string table (.strtab: "\0_start\0main\0")
# 0x0120: Symbol table (.symtab: 3 entries: UNDEF, _start, main)
# 0x0200: Section headers (Elf64_Shdr * 5: NULL, .text, .shstrtab, .symtab, .strtab)

def generate_test_elf(filename="sample_test.elf"):
    # Sections data
    code = b"\x90\xc3\x90\x90" # nop; ret; nop; nop
    
    shstrtab = b"\x00.text\x00.shstrtab\x00.symtab\x00.strtab\x00"
    shstr_text = 1
    shstr_shstrtab = 7
    shstr_symtab = 17
    shstr_strtab = 25
    
    strtab = b"\x00_start\x00main\x00"
    str_start = 1
    str_main = 8

    # Offsets
    ehdr_size = 64
    phdr_size = 56
    shdr_size = 64

    offset_ehdr = 0
    offset_phdr = ehdr_size
    offset_text = 0x80
    offset_shstrtab = 0x100
    offset_strtab = 0x140
    offset_symtab = 0x180
    
    # 3 symbols: 0 = UNDEF, 1 = _start, 2 = main
    # Elf64_Sym: st_name(4), st_info(1), st_other(1), st_shndx(2), st_value(8), st_size(8) -> 24 bytes
    sym_size = 24
    sym0 = struct.pack("<IBBHQQ", 0, 0, 0, 0, 0, 0)
    # _start: GLOBAL FUNC, in section 1 (.text), addr 0x400080, size 4
    # st_info: (STB_GLOBAL << 4) | STT_FUNC = (1 << 4) | 2 = 0x12
    sym1 = struct.pack("<IBBHQQ", str_start, 0x12, 0, 1, 0x400080, 4)
    # main: GLOBAL FUNC, in section 1 (.text), addr 0x400080, size 4
    sym2 = struct.pack("<IBBHQQ", str_main, 0x12, 0, 1, 0x400080, 4)
    symtab_data = sym0 + sym1 + sym2

    offset_shdrs = 0x200

    # Elf64_Ehdr
    e_ident = b"\x7fELF" + bytes([2, 1, 1, 0, 0]) + b"\x00" * 7 # 64-bit, little-endian, EV_CURRENT, System V
    ehdr = struct.pack(
        "<16sHHIQQQIHHHHHH",
        e_ident,
        2,       # e_type = ET_EXEC
        62,      # e_machine = EM_X86_64
        1,       # e_version = 1
        0x400080,# e_entry
        offset_phdr, # e_phoff
        offset_shdrs,# e_shoff
        0,       # e_flags
        ehdr_size,# e_ehsize
        phdr_size,# e_phentsize
        1,       # e_phnum
        shdr_size,# e_shentsize
        5,       # e_shnum
        2        # e_shstrndx (index 2 is .shstrtab)
    )

    # Elf64_Phdr: p_type(4), p_flags(4), p_offset(8), p_vaddr(8), p_paddr(8), p_filesz(8), p_memsz(8), p_align(8)
    # PT_LOAD = 1, PF_R | PF_X = 5
    phdr = struct.pack(
        "<IIQQQQQQ",
        1,       # PT_LOAD
        5,       # PF_R | PF_X
        0,       # offset
        0x400000,# vaddr
        0x400000,# paddr
        0x300,   # filesz
        0x300,   # memsz
        0x1000   # align
    )

    # Section Headers (5 headers)
    # Elf64_Shdr: sh_name(4), sh_type(4), sh_flags(8), sh_addr(8), sh_offset(8), sh_size(8), sh_link(4), sh_info(4), sh_addralign(8), sh_entsize(8)
    # 0: NULL
    shdr0 = struct.pack("<IIQQQQIIQQ", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    # 1: .text (PROGBITS = 1, SHF_ALLOC | SHF_EXECINSTR = 6)
    shdr1 = struct.pack("<IIQQQQIIQQ", shstr_text, 1, 6, 0x400080, offset_text, len(code), 0, 0, 16, 0)
    # 2: .shstrtab (STRTAB = 3)
    shdr2 = struct.pack("<IIQQQQIIQQ", shstr_shstrtab, 3, 0, 0, offset_shstrtab, len(shstrtab), 0, 0, 1, 0)
    # 3: .symtab (SYMTAB = 2, link = 4 (.strtab), info = 1)
    shdr3 = struct.pack("<IIQQQQIIQQ", shstr_symtab, 2, 0, 0, offset_symtab, len(symtab_data), 4, 1, 8, sym_size)
    # 4: .strtab (STRTAB = 3)
    shdr4 = struct.pack("<IIQQQQIIQQ", shstr_strtab, 3, 0, 0, offset_strtab, len(strtab), 0, 0, 1, 0)

    # Assemble full binary
    total_len = offset_shdrs + 5 * shdr_size
    buf = bytearray(total_len)

    buf[offset_ehdr:offset_ehdr + len(ehdr)] = ehdr
    buf[offset_phdr:offset_phdr + len(phdr)] = phdr
    buf[offset_text:offset_text + len(code)] = code
    buf[offset_shstrtab:offset_shstrtab + len(shstrtab)] = shstrtab
    buf[offset_strtab:offset_strtab + len(strtab)] = strtab
    buf[offset_symtab:offset_symtab + len(symtab_data)] = symtab_data

    shdrs = shdr0 + shdr1 + shdr2 + shdr3 + shdr4
    buf[offset_shdrs:offset_shdrs + len(shdrs)] = shdrs

    with open(filename, "wb") as f:
        f.write(buf)

    print(f"Generated {filename} ({len(buf)} bytes)")

if __name__ == "__main__":
    generate_test_elf()
