#include "elf_parser.h"
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <algorithm>

namespace {

std::string getOsAbiStr(uint8_t abi) {
    switch (abi) {
        case ELFOSABI_NONE:       return "UNIX - System V";
        case ELFOSABI_HPUX:       return "HP-UX";
        case ELFOSABI_NETBSD:     return "NetBSD";
        case ELFOSABI_LINUX:      return "Linux";
        case ELFOSABI_SOLARIS:    return "Solaris";
        case ELFOSABI_FREEBSD:    return "FreeBSD";
        case ELFOSABI_OPENBSD:    return "OpenBSD";
        case ELFOSABI_STANDALONE: return "Standalone App";
        default:                  return "Unknown (" + std::to_string(abi) + ")";
    }
}

std::string getTypeStr(uint16_t type) {
    switch (type) {
        case ET_NONE: return "NONE (No file type)";
        case ET_REL:  return "REL (Relocatable object file)";
        case ET_EXEC: return "EXEC (Executable file)";
        case ET_DYN:  return "DYN (Position-Independent Executable or Shared Object)";
        case ET_CORE: return "CORE (Core file)";
        default:      return "Unknown (" + std::to_string(type) + ")";
    }
}

std::string getMachineStr(uint16_t machine) {
    switch (machine) {
        case EM_NONE:    return "None";
        case EM_386:     return "Intel 80386";
        case EM_MIPS:    return "MIPS R3000";
        case EM_PPC:     return "PowerPC";
        case EM_PPC64:   return "PowerPC 64-bit";
        case EM_S390:    return "IBM S/390";
        case EM_ARM:     return "ARM";
        case EM_X86_64:  return "Advanced Micro Devices X86-64";
        case EM_AARCH64: return "AArch64";
        case EM_CUDA:    return "NVIDIA CUDA";
        case EM_RISCV:   return "RISC-V";
        default:         return "Unknown (" + std::to_string(machine) + ")";
    }
}

std::string getSectionTypeStr(uint32_t type) {
    switch (type) {
        case SHT_NULL:          return "NULL";
        case SHT_PROGBITS:      return "PROGBITS";
        case SHT_SYMTAB:        return "SYMTAB";
        case SHT_STRTAB:        return "STRTAB";
        case SHT_RELA:          return "RELA";
        case SHT_HASH:          return "HASH";
        case SHT_DYNAMIC:       return "DYNAMIC";
        case SHT_NOTE:          return "NOTE";
        case SHT_NOBITS:        return "NOBITS";
        case SHT_REL:           return "REL";
        case SHT_SHLIB:         return "SHLIB";
        case SHT_DYNSYM:        return "DYNSYM";
        case SHT_INIT_ARRAY:    return "INIT_ARRAY";
        case SHT_FINI_ARRAY:    return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY: return "PREINIT_ARRAY";
        case SHT_GNU_HASH:      return "GNU_HASH";
        case SHT_GNU_VERSYM:    return "VERSYM";
        case SHT_GNU_VERNEED:   return "VERNEED";
        default: {
            std::stringstream ss;
            ss << "0x" << std::hex << type;
            return ss.str();
        }
    }
}

std::string getSectionFlagsStr(uint64_t flags) {
    std::string s;
    if (flags & SHF_WRITE)     s += 'W';
    if (flags & SHF_ALLOC)     s += 'A';
    if (flags & SHF_EXECINSTR) s += 'X';
    if (flags & SHF_MERGE)     s += 'M';
    if (flags & SHF_STRINGS)   s += 'S';
    if (flags & SHF_INFO_LINK) s += 'I';
    if (flags & SHF_LINK_ORDER)s += 'L';
    if (flags & SHF_TLS)       s += 'T';
    return s;
}

std::string getSegmentTypeStr(uint32_t type) {
    switch (type) {
        case PT_NULL:         return "NULL";
        case PT_LOAD:         return "LOAD";
        case PT_DYNAMIC:      return "DYNAMIC";
        case PT_INTERP:       return "INTERP";
        case PT_NOTE:         return "NOTE";
        case PT_SHLIB:        return "SHLIB";
        case PT_PHDR:         return "PHDR";
        case PT_TLS:          return "TLS";
        case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
        case PT_GNU_STACK:    return "GNU_STACK";
        case PT_GNU_RELRO:    return "GNU_RELRO";
        case PT_GNU_PROPERTY: return "GNU_PROPERTY";
        default: {
            std::stringstream ss;
            ss << "0x" << std::hex << type;
            return ss.str();
        }
    }
}

std::string getSegmentFlagsStr(uint32_t flags) {
    std::string s = "   ";
    if (flags & PF_R) s[0] = 'R';
    if (flags & PF_W) s[1] = 'W';
    if (flags & PF_X) s[2] = 'E';
    return s;
}

std::string getSymbolBindStr(uint8_t bind) {
    switch (bind) {
        case STB_LOCAL:  return "LOCAL";
        case STB_GLOBAL: return "GLOBAL";
        case STB_WEAK:   return "WEAK";
        default:         return "NUM (" + std::to_string(bind) + ")";
    }
}

std::string getSymbolTypeStr(uint8_t type) {
    switch (type) {
        case STT_NOTYPE:  return "NOTYPE";
        case STT_OBJECT:  return "OBJECT";
        case STT_FUNC:    return "FUNC";
        case STT_SECTION: return "SECTION";
        case STT_FILE:    return "FILE";
        case STT_COMMON:  return "COMMON";
        case STT_TLS:     return "TLS";
        default:          return "NUM (" + std::to_string(type) + ")";
    }
}

std::string getSymbolVisStr(uint8_t vis) {
    switch (vis & 0x3) {
        case STV_DEFAULT:   return "DEFAULT";
        case STV_INTERNAL:  return "INTERNAL";
        case STV_HIDDEN:    return "HIDDEN";
        case STV_PROTECTED: return "PROTECTED";
        default:            return "UNKNOWN";
    }
}

std::string getDynamicTagStr(int64_t tag) {
    switch (tag) {
        case DT_NULL:       return "NULL";
        case DT_NEEDED:     return "NEEDED";
        case DT_PLTRELSZ:   return "PLTRELSZ";
        case DT_PLTGOT:     return "PLTGOT";
        case DT_HASH:       return "HASH";
        case DT_STRTAB:     return "STRTAB";
        case DT_SYMTAB:     return "SYMTAB";
        case DT_RELA:       return "RELA";
        case DT_RELASZ:     return "RELASZ";
        case DT_RELAENT:    return "RELAENT";
        case DT_STRSZ:      return "STRSZ";
        case DT_SYMENT:     return "SYMENT";
        case DT_INIT:       return "INIT";
        case DT_FINI:       return "FINI";
        case DT_SONAME:     return "SONAME";
        case DT_RPATH:      return "RPATH";
        case DT_SYMBOLIC:   return "SYMBOLIC";
        case DT_REL:        return "REL";
        case DT_RELSZ:      return "RELSZ";
        case DT_RELENT:     return "RELENT";
        case DT_PLTREL:     return "PLTREL";
        case DT_DEBUG:      return "DEBUG";
        case DT_TEXTREL:    return "TEXTREL";
        case DT_JMPREL:     return "JMPREL";
        case DT_BIND_NOW:   return "BIND_NOW";
        case DT_FLAGS:      return "FLAGS";
        case DT_GNU_HASH:   return "GNU_HASH";
        case DT_VERSYM:     return "VERSYM";
        case DT_FLAGS_1:    return "FLAGS_1";
        case DT_VERNEED:    return "VERNEED";
        case DT_VERNEEDNUM: return "VERNEEDNUM";
        default: {
            std::stringstream ss;
            ss << "0x" << std::hex << tag;
            return ss.str();
        }
    }
}

std::string getRelocationTypeStr(uint16_t machine, uint32_t type) {
    if (machine == EM_X86_64) {
        switch (type) {
            case R_X86_64_NONE:          return "R_X86_64_NONE";
            case R_X86_64_64:            return "R_X86_64_64";
            case R_X86_64_PC32:          return "R_X86_64_PC32";
            case R_X86_64_GOT32:         return "R_X86_64_GOT32";
            case R_X86_64_PLT32:         return "R_X86_64_PLT32";
            case R_X86_64_COPY:          return "R_X86_64_COPY";
            case R_X86_64_GLOB_DAT:      return "R_X86_64_GLOB_DAT";
            case R_X86_64_JUMP_SLOT:     return "R_X86_64_JUMP_SLOT";
            case R_X86_64_RELATIVE:      return "R_X86_64_RELATIVE";
            case R_X86_64_GOTPCREL:      return "R_X86_64_GOTPCREL";
            case R_X86_64_32:            return "R_X86_64_32";
            case R_X86_64_32S:           return "R_X86_64_32S";
            case R_X86_64_PC64:          return "R_X86_64_PC64";
            case R_X86_64_GOTPC32:       return "R_X86_64_GOTPC32";
            case R_X86_64_REX_GOTPCRELX: return "R_X86_64_REX_GOTPCRELX";
            default: break;
        }
    } else if (machine == EM_ARM) {
        switch (type) {
            case R_ARM_NONE:      return "R_ARM_NONE";
            case R_ARM_PC24:      return "R_ARM_PC24";
            case R_ARM_ABS32:     return "R_ARM_ABS32";
            case R_ARM_CALL:      return "R_ARM_CALL";
            case R_ARM_JUMP24:    return "R_ARM_JUMP24";
            case R_ARM_GLOB_DAT:  return "R_ARM_GLOB_DAT";
            case R_ARM_JUMP_SLOT: return "R_ARM_JUMP_SLOT";
            case R_ARM_RELATIVE:  return "R_ARM_RELATIVE";
            case R_ARM_COPY:      return "R_ARM_COPY";
            default: break;
        }
    }
    std::stringstream ss;
    ss << "0x" << std::hex << type;
    return ss.str();
}

} // anonymous namespace

ELFParser::ELFParser() = default;
ELFParser::~ELFParser() = default;

uint16_t ELFParser::read16(const void* ptr) const {
    uint16_t val;
    std::memcpy(&val, ptr, sizeof(val));
    if (!is_little_endian_) {
        val = (val >> 8) | (val << 8);
    }
    return val;
}

uint32_t ELFParser::read32(const void* ptr) const {
    uint32_t val;
    std::memcpy(&val, ptr, sizeof(val));
    if (!is_little_endian_) {
        val = ((val >> 24) & 0x000000ff) |
              ((val >> 8)  & 0x0000ff00) |
              ((val << 8)  & 0x00ff0000) |
              ((val << 24) & 0xff000000);
    }
    return val;
}

uint64_t ELFParser::read64(const void* ptr) const {
    uint64_t val;
    std::memcpy(&val, ptr, sizeof(val));
    if (!is_little_endian_) {
        val = ((val >> 56) & 0x00000000000000ffULL) |
              ((val >> 40) & 0x000000000000ff00ULL) |
              ((val >> 24) & 0x0000000000ff0000ULL) |
              ((val >> 8)  & 0x00000000ff000000ULL) |
              ((val << 8)  & 0x000000ff00000000ULL) |
              ((val << 24) & 0x0000ff0000000000ULL) |
              ((val << 40) & 0x00ff000000000000ULL) |
              ((val << 56) & 0xff00000000000000ULL);
    }
    return val;
}

std::string ELFParser::color(const std::string& code, const std::string& text) const {
    if (!use_color_) return text;
    return "\033[" + code + "m" + text + "\033[0m";
}

bool ELFParser::load(const std::string& filepath) {
    filepath_ = filepath;
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << color("31;1", "Error: ") << "Cannot open file " << filepath << "\n";
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size < EI_NIDENT) {
        std::cerr << color("31;1", "Error: ") << "File is too small to be a valid ELF binary.\n";
        return false;
    }

    buffer_.resize(size);
    if (!file.read(reinterpret_cast<char*>(buffer_.data()), size)) {
        std::cerr << color("31;1", "Error: ") << "Failed to read file into buffer.\n";
        return false;
    }

    if (!parseIdentification()) return false;
    if (!parseHeaders()) return false;
    if (!parseSections()) return false;
    if (!parseSegments()) return false;
    parseSymbols();
    parseRelocations();
    parseDynamic();

    return true;
}

bool ELFParser::parseIdentification() {
    if (buffer_[EI_MAG0] != ELFMAG0 ||
        buffer_[EI_MAG1] != ELFMAG1 ||
        buffer_[EI_MAG2] != ELFMAG2 ||
        buffer_[EI_MAG3] != ELFMAG3) {
        std::cerr << color("31;1", "Error: ") << "Not an ELF file (magic mismatch: "
                  << std::hex << (int)buffer_[0] << " " << (int)buffer_[1] << " "
                  << (int)buffer_[2] << " " << (int)buffer_[3] << std::dec << ")\n";
        return false;
    }

    uint8_t elf_class = buffer_[EI_CLASS];
    if (elf_class == ELFCLASS32) {
        is_64bit_ = false;
    } else if (elf_class == ELFCLASS64) {
        is_64bit_ = true;
    } else {
        std::cerr << color("31;1", "Error: ") << "Invalid ELF class: " << (int)elf_class << "\n";
        return false;
    }

    uint8_t elf_data = buffer_[EI_DATA];
    if (elf_data == ELFDATA2LSB) {
        is_little_endian_ = true;
    } else if (elf_data == ELFDATA2MSB) {
        is_little_endian_ = false;
    } else {
        std::cerr << color("31;1", "Error: ") << "Invalid data encoding (endianness): " << (int)elf_data << "\n";
        return false;
    }

    e_osabi_ = buffer_[EI_OSABI];
    e_abiversion_ = buffer_[EI_ABIVERSION];

    return true;
}

bool ELFParser::parseHeaders() {
    if (is_64bit_) {
        if (buffer_.size() < sizeof(Elf64_Ehdr)) return false;
        const auto* ehdr = reinterpret_cast<const Elf64_Ehdr*>(buffer_.data());
        e_type_ = read16(&ehdr->e_type);
        e_machine_ = read16(&ehdr->e_machine);
        e_version_ = read32(&ehdr->e_version);
        e_entry_ = read64(&ehdr->e_entry);
        e_phoff_ = read64(&ehdr->e_phoff);
        e_shoff_ = read64(&ehdr->e_shoff);
        e_flags_ = read32(&ehdr->e_flags);
        e_ehsize_ = read16(&ehdr->e_ehsize);
        e_phentsize_ = read16(&ehdr->e_phentsize);
        e_phnum_ = read16(&ehdr->e_phnum);
        e_shentsize_ = read16(&ehdr->e_shentsize);
        e_shnum_ = read16(&ehdr->e_shnum);
        e_shstrndx_ = read16(&ehdr->e_shstrndx);
    } else {
        if (buffer_.size() < sizeof(Elf32_Ehdr)) return false;
        const auto* ehdr = reinterpret_cast<const Elf32_Ehdr*>(buffer_.data());
        e_type_ = read16(&ehdr->e_type);
        e_machine_ = read16(&ehdr->e_machine);
        e_version_ = read32(&ehdr->e_version);
        e_entry_ = read32(&ehdr->e_entry);
        e_phoff_ = read32(&ehdr->e_phoff);
        e_shoff_ = read32(&ehdr->e_shoff);
        e_flags_ = read32(&ehdr->e_flags);
        e_ehsize_ = read16(&ehdr->e_ehsize);
        e_phentsize_ = read16(&ehdr->e_phentsize);
        e_phnum_ = read16(&ehdr->e_phnum);
        e_shentsize_ = read16(&ehdr->e_shentsize);
        e_shnum_ = read16(&ehdr->e_shnum);
        e_shstrndx_ = read16(&ehdr->e_shstrndx);
    }
    return true;
}

std::string ELFParser::getStringFromTable(uint64_t strtab_offset, uint64_t strtab_size, uint32_t str_index) const {
    if (str_index >= strtab_size || strtab_offset + str_index >= buffer_.size()) {
        return "";
    }
    const char* str = reinterpret_cast<const char*>(buffer_.data() + strtab_offset + str_index);
    size_t max_len = std::min<size_t>(strtab_size - str_index, buffer_.size() - (strtab_offset + str_index));
    size_t len = 0;
    while (len < max_len && str[len] != '\0') {
        len++;
    }
    return std::string(str, len);
}

bool ELFParser::parseSections() {
    sections_.clear();
    if (e_shoff_ == 0 || e_shnum_ == 0) return true;

    sections_.resize(e_shnum_);

    // 1st Pass: Parse raw structures
    for (uint32_t i = 0; i < e_shnum_; ++i) {
        uint64_t offset = e_shoff_ + (uint64_t)i * e_shentsize_;
        if (offset + (is_64bit_ ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr)) > buffer_.size()) {
            std::cerr << color("33", "Warning: ") << "Section header " << i << " is out of bounds.\n";
            continue;
        }

        SectionInfo& sec = sections_[i];
        sec.index = i;

        if (is_64bit_) {
            const auto* shdr = reinterpret_cast<const Elf64_Shdr*>(buffer_.data() + offset);
            uint32_t name_idx = read32(&shdr->sh_name);
            sec.type = read32(&shdr->sh_type);
            sec.flags = read64(&shdr->sh_flags);
            sec.addr = read64(&shdr->sh_addr);
            sec.offset = read64(&shdr->sh_offset);
            sec.size = read64(&shdr->sh_size);
            sec.link = read32(&shdr->sh_link);
            sec.info = read32(&shdr->sh_info);
            sec.addralign = read64(&shdr->sh_addralign);
            sec.entsize = read64(&shdr->sh_entsize);
            sec.name = std::to_string(name_idx); // temporarily store index
        } else {
            const auto* shdr = reinterpret_cast<const Elf32_Shdr*>(buffer_.data() + offset);
            uint32_t name_idx = read32(&shdr->sh_name);
            sec.type = read32(&shdr->sh_type);
            sec.flags = read32(&shdr->sh_flags);
            sec.addr = read32(&shdr->sh_addr);
            sec.offset = read32(&shdr->sh_offset);
            sec.size = read32(&shdr->sh_size);
            sec.link = read32(&shdr->sh_link);
            sec.info = read32(&shdr->sh_info);
            sec.addralign = read32(&shdr->sh_addralign);
            sec.entsize = read32(&shdr->sh_entsize);
            sec.name = std::to_string(name_idx); // temporarily store index
        }

        sec.type_str = getSectionTypeStr(sec.type);
        sec.flags_str = getSectionFlagsStr(sec.flags);
    }

    // 2nd Pass: Resolve section names using .shstrtab
    if (e_shstrndx_ < sections_.size()) {
        const SectionInfo& strtab_sec = sections_[e_shstrndx_];
        for (auto& sec : sections_) {
            uint32_t name_idx = std::stoul(sec.name);
            sec.name = getStringFromTable(strtab_sec.offset, strtab_sec.size, name_idx);
        }
    }

    return true;
}

bool ELFParser::parseSegments() {
    segments_.clear();
    if (e_phoff_ == 0 || e_phnum_ == 0) return true;

    segments_.resize(e_phnum_);

    for (uint32_t i = 0; i < e_phnum_; ++i) {
        uint64_t offset = e_phoff_ + (uint64_t)i * e_phentsize_;
        if (offset + (is_64bit_ ? sizeof(Elf64_Phdr) : sizeof(Elf32_Phdr)) > buffer_.size()) {
            std::cerr << color("33", "Warning: ") << "Program header " << i << " is out of bounds.\n";
            continue;
        }

        SegmentInfo& seg = segments_[i];
        seg.index = i;

        if (is_64bit_) {
            const auto* phdr = reinterpret_cast<const Elf64_Phdr*>(buffer_.data() + offset);
            seg.type = read32(&phdr->p_type);
            seg.flags = read32(&phdr->p_flags);
            seg.offset = read64(&phdr->p_offset);
            seg.vaddr = read64(&phdr->p_vaddr);
            seg.paddr = read64(&phdr->p_paddr);
            seg.filesz = read64(&phdr->p_filesz);
            seg.memsz = read64(&phdr->p_memsz);
            seg.align = read64(&phdr->p_align);
        } else {
            const auto* phdr = reinterpret_cast<const Elf32_Phdr*>(buffer_.data() + offset);
            seg.type = read32(&phdr->p_type);
            seg.offset = read32(&phdr->p_offset);
            seg.vaddr = read32(&phdr->p_vaddr);
            seg.paddr = read32(&phdr->p_paddr);
            seg.filesz = read32(&phdr->p_filesz);
            seg.memsz = read32(&phdr->p_memsz);
            seg.flags = read32(&phdr->p_flags);
            seg.align = read32(&phdr->p_align);
        }

        seg.type_str = getSegmentTypeStr(seg.type);
        seg.flags_str = getSegmentFlagsStr(seg.flags);
    }

    return true;
}

bool ELFParser::parseSymbols() {
    symbols_.clear();
    dyn_symbols_.clear();

    auto parseSymTable = [&](const SectionInfo& sym_sec, std::vector<SymbolInfo>& out_symbols) {
        if (sym_sec.link >= sections_.size()) return;
        const SectionInfo& strtab_sec = sections_[sym_sec.link];

        size_t entry_size = is_64bit_ ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym);
        if (sym_sec.entsize != 0) entry_size = sym_sec.entsize;
        if (entry_size == 0) return;

        size_t num_symbols = sym_sec.size / entry_size;
        out_symbols.reserve(num_symbols);

        for (size_t i = 0; i < num_symbols; ++i) {
            uint64_t offset = sym_sec.offset + i * entry_size;
            if (offset + entry_size > buffer_.size()) break;

            SymbolInfo sym;
            sym.index = static_cast<uint32_t>(i);

            uint32_t name_idx = 0;
            if (is_64bit_) {
                const auto* s = reinterpret_cast<const Elf64_Sym*>(buffer_.data() + offset);
                name_idx = read32(&s->st_name);
                sym.bind = ELF64_ST_BIND(s->st_info);
                sym.type = ELF64_ST_TYPE(s->st_info);
                sym.vis = s->st_other;
                sym.shndx = read16(&s->st_shndx);
                sym.value = read64(&s->st_value);
                sym.size = read64(&s->st_size);
            } else {
                const auto* s = reinterpret_cast<const Elf32_Sym*>(buffer_.data() + offset);
                name_idx = read32(&s->st_name);
                sym.value = read32(&s->st_value);
                sym.size = read32(&s->st_size);
                sym.bind = ELF32_ST_BIND(s->st_info);
                sym.type = ELF32_ST_TYPE(s->st_info);
                sym.vis = s->st_other;
                sym.shndx = read16(&s->st_shndx);
            }

            sym.name = getStringFromTable(strtab_sec.offset, strtab_sec.size, name_idx);
            sym.bind_str = getSymbolBindStr(sym.bind);
            sym.type_str = getSymbolTypeStr(sym.type);
            sym.vis_str = getSymbolVisStr(sym.vis);

            if (sym.shndx == SHN_UNDEF) sym.shndx_str = "UND";
            else if (sym.shndx == SHN_ABS) sym.shndx_str = "ABS";
            else if (sym.shndx == SHN_COMMON) sym.shndx_str = "COM";
            else sym.shndx_str = std::to_string(sym.shndx);

            out_symbols.push_back(sym);
        }
    };

    for (const auto& sec : sections_) {
        if (sec.type == SHT_SYMTAB) {
            parseSymTable(sec, symbols_);
        } else if (sec.type == SHT_DYNSYM) {
            parseSymTable(sec, dyn_symbols_);
        }
    }

    return true;
}

bool ELFParser::parseRelocations() {
    relocations_.clear();

    for (const auto& sec : sections_) {
        if (sec.type != SHT_REL && sec.type != SHT_RELA) continue;

        std::vector<RelocationInfo> rels;
        bool has_addend = (sec.type == SHT_RELA);

        size_t entry_size = 0;
        if (is_64bit_) {
            entry_size = has_addend ? sizeof(Elf64_Rela) : sizeof(Elf64_Rel);
        } else {
            entry_size = has_addend ? sizeof(Elf32_Rela) : sizeof(Elf32_Rel);
        }
        if (sec.entsize != 0) entry_size = sec.entsize;
        if (entry_size == 0) continue;

        const std::vector<SymbolInfo>* sym_table = nullptr;
        if (sec.link < sections_.size()) {
            if (sections_[sec.link].type == SHT_DYNSYM) sym_table = &dyn_symbols_;
            else if (sections_[sec.link].type == SHT_SYMTAB) sym_table = &symbols_;
        }

        size_t num_rels = sec.size / entry_size;
        for (size_t i = 0; i < num_rels; ++i) {
            uint64_t offset = sec.offset + i * entry_size;
            if (offset + entry_size > buffer_.size()) break;

            RelocationInfo r;
            r.has_addend = has_addend;

            if (is_64bit_) {
                if (has_addend) {
                    const auto* ra = reinterpret_cast<const Elf64_Rela*>(buffer_.data() + offset);
                    r.offset = read64(&ra->r_offset);
                    r.info = read64(&ra->r_info);
                    r.addend = static_cast<int64_t>(read64(&ra->r_addend));
                } else {
                    const auto* rel = reinterpret_cast<const Elf64_Rel*>(buffer_.data() + offset);
                    r.offset = read64(&rel->r_offset);
                    r.info = read64(&rel->r_info);
                    r.addend = 0;
                }
                r.sym_index = static_cast<uint32_t>(ELF64_R_SYM(r.info));
                r.rel_type = static_cast<uint32_t>(ELF64_R_TYPE(r.info));
            } else {
                if (has_addend) {
                    const auto* ra = reinterpret_cast<const Elf32_Rela*>(buffer_.data() + offset);
                    r.offset = read32(&ra->r_offset);
                    r.info = read32(&ra->r_info);
                    r.addend = static_cast<int32_t>(read32(&ra->r_addend));
                } else {
                    const auto* rel = reinterpret_cast<const Elf32_Rel*>(buffer_.data() + offset);
                    r.offset = read32(&rel->r_offset);
                    r.info = read32(&rel->r_info);
                    r.addend = 0;
                }
                r.sym_index = ELF32_R_SYM(r.info);
                r.rel_type = ELF32_R_TYPE(r.info);
            }

            r.type_str = getRelocationTypeStr(e_machine_, r.rel_type);

            if (sym_table && r.sym_index < sym_table->size()) {
                r.sym_name = (*sym_table)[r.sym_index].name;
            } else {
                r.sym_name = "";
            }

            rels.push_back(r);
        }

        relocations_.emplace_back(sec.name, std::move(rels));
    }

    return true;
}

bool ELFParser::parseDynamic() {
    dynamic_entries_.clear();

    const SectionInfo* dyn_sec = nullptr;
    for (const auto& sec : sections_) {
        if (sec.type == SHT_DYNAMIC) {
            dyn_sec = &sec;
            break;
        }
    }
    if (!dyn_sec) return true;

    const SectionInfo* dynstr_sec = nullptr;
    if (dyn_sec->link < sections_.size()) {
        dynstr_sec = &sections_[dyn_sec->link];
    }

    size_t entry_size = is_64bit_ ? sizeof(Elf64_Dyn) : sizeof(Elf32_Dyn);
    if (dyn_sec->entsize != 0) entry_size = dyn_sec->entsize;
    if (entry_size == 0) return true;

    size_t count = dyn_sec->size / entry_size;
    for (size_t i = 0; i < count; ++i) {
        uint64_t offset = dyn_sec->offset + i * entry_size;
        if (offset + entry_size > buffer_.size()) break;

        DynamicInfo d;
        if (is_64bit_) {
            const auto* dyn = reinterpret_cast<const Elf64_Dyn*>(buffer_.data() + offset);
            d.tag = static_cast<int64_t>(read64(&dyn->d_tag));
            d.val = read64(&dyn->d_un.d_val);
        } else {
            const auto* dyn = reinterpret_cast<const Elf32_Dyn*>(buffer_.data() + offset);
            d.tag = static_cast<int32_t>(read32(&dyn->d_tag));
            d.val = read32(&dyn->d_un.d_val);
        }

        d.tag_str = getDynamicTagStr(d.tag);

        if ((d.tag == DT_NEEDED || d.tag == DT_SONAME || d.tag == DT_RPATH) && dynstr_sec) {
            d.val_str = "Shared library: [" + getStringFromTable(dynstr_sec->offset, dynstr_sec->size, static_cast<uint32_t>(d.val)) + "]";
        } else {
            std::stringstream ss;
            ss << "0x" << std::hex << d.val;
            d.val_str = ss.str();
        }

        dynamic_entries_.push_back(d);
        if (d.tag == DT_NULL) break;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Print functions
// ─────────────────────────────────────────────────────────────────────────────

void ELFParser::printELFHeader() const {
    std::cout << color("1;36", "ELF Header:") << "\n";
    std::cout << "  Magic:   ";
    for (int i = 0; i < EI_NIDENT; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer_[i] << " ";
    }
    std::cout << std::dec << "\n";

    std::cout << "  Class:                             " << (is_64bit_ ? "ELF64" : "ELF32") << "\n";
    std::cout << "  Data:                              "
              << (is_little_endian_ ? "2's complement, little endian" : "2's complement, big endian") << "\n";
    std::cout << "  Version:                           " << (int)buffer_[EI_VERSION] << " (current)\n";
    std::cout << "  OS/ABI:                            " << getOsAbiStr(e_osabi_) << "\n";
    std::cout << "  ABI Version:                       " << (int)e_abiversion_ << "\n";
    std::cout << "  Type:                              " << getTypeStr(e_type_) << "\n";
    std::cout << "  Machine:                           " << getMachineStr(e_machine_) << "\n";
    std::cout << "  Version:                           0x" << std::hex << e_version_ << std::dec << "\n";
    std::cout << "  Entry point address:               " << color("32", "0x" + [this](){
        std::stringstream ss; ss << std::hex << e_entry_; return ss.str();
    }()) << "\n";
    std::cout << "  Start of program headers:          " << e_phoff_ << " (bytes into file)\n";
    std::cout << "  Start of section headers:          " << e_shoff_ << " (bytes into file)\n";
    std::cout << "  Flags:                             0x" << std::hex << e_flags_ << std::dec << "\n";
    std::cout << "  Size of this header:               " << e_ehsize_ << " (bytes)\n";
    std::cout << "  Size of program headers:           " << e_phentsize_ << " (bytes)\n";
    std::cout << "  Number of program headers:         " << e_phnum_ << "\n";
    std::cout << "  Size of section headers:           " << e_shentsize_ << " (bytes)\n";
    std::cout << "  Number of section headers:         " << e_shnum_ << "\n";
    std::cout << "  Section header string table index: " << e_shstrndx_ << "\n";
}

void ELFParser::printProgramHeaders() const {
    if (segments_.empty()) {
        std::cout << "\nThere are no program headers in this file.\n";
        return;
    }

    std::cout << "\n" << color("1;36", "Program Headers:") << "\n";
    std::cout << "  Type           Offset             VirtAddr           PhysAddr\n"
              << "                 FileSiz            MemSiz              Flags  Align\n";

    for (const auto& seg : segments_) {
        std::cout << "  " << std::left << std::setw(14) << std::setfill(' ') << seg.type_str
                  << " 0x" << std::hex << std::right << std::setw(16) << std::setfill('0') << seg.offset
                  << " 0x" << std::setw(16) << seg.vaddr
                  << " 0x" << std::setw(16) << seg.paddr << "\n"
                  << "                 0x" << std::setw(16) << seg.filesz
                  << " 0x" << std::setw(16) << seg.memsz
                  << "  " << std::setfill(' ') << std::left << std::setw(6) << color("33", seg.flags_str)
                  << " 0x" << std::hex << seg.align << std::dec << "\n";
    }
}

void ELFParser::printSectionHeaders() const {
    if (sections_.empty()) {
        std::cout << "\nThere are no sections in this file.\n";
        return;
    }

    std::cout << "\n" << color("1;36", "Section Headers:") << "\n";
    std::cout << "  [Nr] Name              Type             Address          Off      Size     ES Flg Lk Inf Al\n";

    for (const auto& sec : sections_) {
        std::cout << "  [" << std::right << std::setw(2) << std::setfill(' ') << std::dec << sec.index << "] "
                  << std::left << std::setw(17) << (sec.name.length() > 17 ? sec.name.substr(0, 16) + "~" : sec.name)
                  << " " << std::setw(16) << (sec.type_str.length() > 16 ? sec.type_str.substr(0, 15) + "~" : sec.type_str)
                  << " " << std::hex << std::right << std::setw(16) << std::setfill('0') << sec.addr
                  << " " << std::setw(8) << sec.offset
                  << " " << std::setw(8) << sec.size
                  << " " << std::setw(2) << sec.entsize
                  << " " << std::setfill(' ') << std::left << std::setw(3) << color("32", sec.flags_str)
                  << " " << std::right << std::setw(2) << std::dec << sec.link
                  << " " << std::setw(3) << sec.info
                  << " " << std::setw(2) << sec.addralign << "\n";
    }
    std::cout << "Key to Flags:\n"
              << "  W (write), A (alloc), X (execute), M (merge), S (strings), I (info), L (link order), T (TLS)\n";
}

void ELFParser::printSymbolTable(bool dynamic_only) const {
    auto printTable = [this](const std::string& title, const std::vector<SymbolInfo>& syms) {
        if (syms.empty()) return;
        std::cout << "\n" << color("1;36", title) << " (" << syms.size() << " symbols):\n";
        std::cout << "   Num:    Value          Size Type    Bind   Vis      Ndx Name\n";

        for (const auto& sym : syms) {
            std::cout << "  " << std::right << std::setw(4) << sym.index << ": "
                      << std::hex << std::setw(16) << std::setfill('0') << sym.value << " "
                      << std::dec << std::setfill(' ') << std::setw(5) << sym.size << " "
                      << std::left << std::setw(7) << sym.type_str << " "
                      << std::setw(6) << sym.bind_str << " "
                      << std::setw(8) << sym.vis_str << " "
                      << std::right << std::setw(4) << sym.shndx_str << " "
                      << color("33", sym.name) << "\n";
        }
    };

    if (!dynamic_only) {
        printTable("Symbol table '.symtab'", symbols_);
    }
    printTable("Symbol table '.dynsym'", dyn_symbols_);
}

void ELFParser::printRelocations() const {
    if (relocations_.empty()) {
        std::cout << "\nThere are no relocations in this file.\n";
        return;
    }

    for (const auto& entry : relocations_) {
        const auto& sec_name = entry.first;
        const auto& rels = entry.second;
        std::cout << "\n" << color("1;36", "Relocation section '" + sec_name + "'")
                  << " at offset contains " << rels.size() << " entries:\n";
        std::cout << "  Offset          Info           Type                          Sym. Name + Addend\n";

        for (const auto& r : rels) {
            std::cout << "  " << std::hex << std::setw(14) << std::setfill('0') << r.offset << "  "
                      << std::setw(14) << r.info << " "
                      << std::setfill(' ') << std::left << std::setw(30) << r.type_str;

            if (!r.sym_name.empty()) {
                std::cout << " " << color("33", r.sym_name);
            }
            if (r.has_addend) {
                if (r.addend >= 0) std::cout << " + " << std::hex << r.addend;
                else std::cout << " - " << std::hex << -r.addend;
            }
            std::cout << std::dec << "\n";
        }
    }
}

void ELFParser::printDynamicSection() const {
    if (dynamic_entries_.empty()) {
        std::cout << "\nThere is no dynamic section in this file.\n";
        return;
    }

    std::cout << "\n" << color("1;36", "Dynamic section") << " contains " << dynamic_entries_.size() << " entries:\n";
    std::cout << "  Tag        Type                         Name/Value\n";

    for (const auto& d : dynamic_entries_) {
        std::cout << "  0x" << std::hex << std::setw(8) << std::setfill('0') << d.tag << " "
                  << std::setfill(' ') << std::left << std::setw(28) << d.tag_str << " "
                  << color("32", d.val_str) << "\n";
    }
}

void ELFParser::printNotes() const {
    bool found = false;
    for (const auto& sec : sections_) {
        if (sec.type == SHT_NOTE) {
            found = true;
            std::cout << "\n" << color("1;36", "Displaying notes found in: " + sec.name) << "\n";
            std::cout << "  Offset: 0x" << std::hex << sec.offset << ", Size: 0x" << sec.size << std::dec << "\n";
        }
    }
    if (!found) {
        std::cout << "\nThere are no note sections in this file.\n";
    }
}

void ELFParser::printHexDump(const std::string& section_name) const {
    const SectionInfo* target = nullptr;
    for (const auto& sec : sections_) {
        if (sec.name == section_name) {
            target = &sec;
            break;
        }
    }

    if (!target) {
        std::cerr << color("31;1", "Error: ") << "Section '" << section_name << "' was not found.\n";
        return;
    }

    if (target->type == SHT_NOBITS) {
        std::cout << "Section '" << section_name << "' has no data to dump (NOBITS).\n";
        return;
    }

    std::cout << "\n" << color("1;36", "Hex dump of section '" + section_name + "':") << "\n";
    uint64_t start = target->offset;
    uint64_t size = target->size;
    uint64_t addr = target->addr;

    for (uint64_t i = 0; i < size; i += 16) {
        std::cout << "  0x" << std::hex << std::setw(8) << std::setfill('0') << (addr + i) << " ";

        // Hex bytes
        for (uint64_t j = 0; j < 16; ++j) {
            if (i + j < size && start + i + j < buffer_.size()) {
                std::cout << std::setw(2) << (int)buffer_[start + i + j] << " ";
            } else {
                std::cout << "   ";
            }
            if (j == 7) std::cout << " ";
        }

        std::cout << " ";

        // ASCII representation
        for (uint64_t j = 0; j < 16; ++j) {
            if (i + j < size && start + i + j < buffer_.size()) {
                uint8_t c = buffer_[start + i + j];
                if (c >= 32 && c <= 126) std::cout << (char)c;
                else std::cout << ".";
            }
        }
        std::cout << "\n";
    }
    std::cout << std::dec;
}

void ELFParser::printSummary() const {
    printELFHeader();
    printProgramHeaders();
    printSectionHeaders();
    if (!symbols_.empty() || !dyn_symbols_.empty()) {
        std::cout << "\nUse -s or --symbols to inspect symbol tables ("
                  << symbols_.size() << " local/global symbols, "
                  << dyn_symbols_.size() << " dynamic symbols).\n";
    }
}

void ELFParser::printAll() const {
    printELFHeader();
    printProgramHeaders();
    printSectionHeaders();
    printDynamicSection();
    printRelocations();
    printSymbolTable();
    printNotes();
}
