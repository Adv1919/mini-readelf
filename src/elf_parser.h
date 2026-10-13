#pragma once

#include "elf_types.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

struct SectionInfo {
    uint32_t index;
    std::string name;
    uint32_t type;
    std::string type_str;
    uint64_t flags;
    std::string flags_str;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entsize;
};

struct SegmentInfo {
    uint32_t index;
    uint32_t type;
    std::string type_str;
    uint32_t flags;
    std::string flags_str;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
};

struct SymbolInfo {
    uint32_t index;
    std::string name;
    uint64_t value;
    uint64_t size;
    uint8_t bind;
    std::string bind_str;
    uint8_t type;
    std::string type_str;
    uint8_t vis;
    std::string vis_str;
    uint16_t shndx;
    std::string shndx_str;
};

struct RelocationInfo {
    uint64_t offset;
    uint64_t info;
    int64_t addend;
    bool has_addend;
    uint32_t sym_index;
    std::string sym_name;
    uint32_t rel_type;
    std::string type_str;
};

struct DynamicInfo {
    int64_t tag;
    std::string tag_str;
    uint64_t val;
    std::string val_str;
};

class ELFParser {
public:
    ELFParser();
    ~ELFParser();

    bool load(const std::string& filepath);

    void setUseColor(bool enable) { use_color_ = enable; }

    void printELFHeader() const;
    void printProgramHeaders() const;
    void printSectionHeaders() const;
    void printSymbolTable(bool dynamic_only = false) const;
    void printRelocations() const;
    void printDynamicSection() const;
    void printNotes() const;
    void printHexDump(const std::string& section_name) const;
    void printSummary() const;
    void printAll() const;

private:
    bool parseIdentification();
    bool parseHeaders();
    bool parseSections();
    bool parseSegments();
    bool parseSymbols();
    bool parseRelocations();
    bool parseDynamic();

    std::string getStringFromTable(uint64_t strtab_offset, uint64_t strtab_size, uint32_t str_index) const;

    // Byte reading helpers taking endianness into account
    uint16_t read16(const void* ptr) const;
    uint32_t read32(const void* ptr) const;
    uint64_t read64(const void* ptr) const;

    // Formatting helpers
    std::string color(const std::string& code, const std::string& text) const;

private:
    std::string filepath_;
    std::vector<uint8_t> buffer_;

    bool is_64bit_ = false;
    bool is_little_endian_ = true;

    // Header info normalized
    uint16_t e_type_ = 0;
    uint16_t e_machine_ = 0;
    uint32_t e_version_ = 0;
    uint64_t e_entry_ = 0;
    uint64_t e_phoff_ = 0;
    uint64_t e_shoff_ = 0;
    uint32_t e_flags_ = 0;
    uint16_t e_ehsize_ = 0;
    uint16_t e_phentsize_ = 0;
    uint16_t e_phnum_ = 0;
    uint16_t e_shentsize_ = 0;
    uint16_t e_shnum_ = 0;
    uint16_t e_shstrndx_ = 0;
    uint8_t  e_osabi_ = 0;
    uint8_t  e_abiversion_ = 0;

    std::vector<SectionInfo> sections_;
    std::vector<SegmentInfo> segments_;
    std::vector<SymbolInfo> symbols_;
    std::vector<SymbolInfo> dyn_symbols_;
    std::vector<std::pair<std::string, std::vector<RelocationInfo>>> relocations_;
    std::vector<DynamicInfo> dynamic_entries_;

    bool use_color_ = true;
};
