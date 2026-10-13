#include "elf_parser.h"
#include <iostream>
#include <vector>
#include <string>

void printUsage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options] <elf-file>\n"
              << "Display information about the contents of ELF format files\n\n"
              << "Options:\n"
              << "  -a, --all               Equivalent to: -h -l -S -s -r -d -n\n"
              << "  -h, --file-header       Display the ELF file header\n"
              << "  -l, --program-headers   Display the program headers (segments)\n"
              << "      --segments          An alias for --program-headers\n"
              << "  -S, --section-headers   Display the sections' header\n"
              << "      --sections          An alias for --section-headers\n"
              << "  -s, --symbols           Display the symbol table\n"
              << "      --syms              An alias for --symbols\n"
              << "      --dyn-syms          Display the dynamic symbol table only\n"
              << "  -r, --relocs            Display the relocations (if present)\n"
              << "  -d, --dynamic           Display the dynamic section (if present)\n"
              << "  -n, --notes             Display the core notes (if present)\n"
              << "  -x, --hex-dump=<name>   Dump the contents of section <name> as bytes\n"
              << "      --no-color          Disable colored output\n"
              << "  -H, --help              Display this information\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string filepath;
    bool show_all = false;
    bool show_header = false;
    bool show_segments = false;
    bool show_sections = false;
    bool show_symbols = false;
    bool show_dyn_syms = false;
    bool show_relocs = false;
    bool show_dynamic = false;
    bool show_notes = false;
    bool use_color = true;
    std::vector<std::string> hex_dump_sections;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--file-header") {
            show_header = true;
        } else if (arg == "-l" || arg == "--program-headers" || arg == "--segments") {
            show_segments = true;
        } else if (arg == "-S" || arg == "--section-headers" || arg == "--sections") {
            show_sections = true;
        } else if (arg == "-s" || arg == "--symbols" || arg == "--syms") {
            show_symbols = true;
        } else if (arg == "--dyn-syms") {
            show_dyn_syms = true;
        } else if (arg == "-r" || arg == "--relocs") {
            show_relocs = true;
        } else if (arg == "-d" || arg == "--dynamic") {
            show_dynamic = true;
        } else if (arg == "-n" || arg == "--notes") {
            show_notes = true;
        } else if (arg == "-a" || arg == "--all") {
            show_all = true;
        } else if (arg == "--no-color") {
            use_color = false;
        } else if (arg == "-H" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg.rfind("-x", 0) == 0) {
            if (arg.length() > 2) {
                if (arg[2] == '=') {
                    hex_dump_sections.push_back(arg.substr(3));
                } else {
                    hex_dump_sections.push_back(arg.substr(2));
                }
            } else if (i + 1 < argc) {
                hex_dump_sections.push_back(argv[++i]);
            }
        } else if (arg.rfind("--hex-dump=", 0) == 0) {
            hex_dump_sections.push_back(arg.substr(11));
        } else if (!arg.empty() && arg[0] != '-') {
            filepath = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (filepath.empty()) {
        std::cerr << "Error: No input file specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    ELFParser parser;
    parser.setUseColor(use_color);

    if (!parser.load(filepath)) {
        return 1;
    }

    bool any_action = show_all || show_header || show_segments || show_sections ||
                      show_symbols || show_dyn_syms || show_relocs || show_dynamic ||
                      show_notes || !hex_dump_sections.empty();

    if (!any_action) {
        // Default summary view if no specific flags were requested
        parser.printSummary();
        return 0;
    }

    if (show_all) {
        parser.printAll();
    } else {
        if (show_header) parser.printELFHeader();
        if (show_segments) parser.printProgramHeaders();
        if (show_sections) parser.printSectionHeaders();
        if (show_dynamic) parser.printDynamicSection();
        if (show_relocs) parser.printRelocations();
        if (show_symbols) parser.printSymbolTable(false);
        else if (show_dyn_syms) parser.printSymbolTable(true);
        if (show_notes) parser.printNotes();
    }

    for (const auto& sec_name : hex_dump_sections) {
        parser.printHexDump(sec_name);
    }

    return 0;
}
