# Mini-Readelf

A fast, lightweight C++17 command-line tool that inspects and parses **ELF (Executable and Linkable Format)** binaries without relying on system headers (`<elf.h>`). Compiles out of the box on Windows, macOS, and Linux.

---

## Features

- **Zero Dependencies**: Self-contained ELF32/ELF64 header definitions; no `<elf.h>` required.
- **Header & Segment Analysis**: Inspects the ELF file header (`-h`) and memory segment mappings (`-l`).
- **Section Tables**: Resolves sections (`.text`, `.data`, `.rodata`, `.bss`) via two-pass string table lookups (`-S`).
- **Symbol Table Decoder**: Dumps local, global, and dynamic symbols (`.symtab` / `.dynsym`) with type and binding information (`-s`).
- **Relocations & Dynamic Info**: Parses relocation entries (`-r`) and dynamic dependencies like shared libraries (`-d`).
- **Hex & ASCII Dump**: Interactively inspects raw bytes of any section (`-x <section>`).
- **Architecture Support**: Handles x86-64, ARM, AArch64, RISC-V, and NVIDIA CUDA binaries (`EM_CUDA`).

---

## Build

### Direct Compile (g++ / MinGW / Clang)
```bash
g++ -std=c++17 -O2 src/elf_parser.cpp src/main.cpp -o elf-parser
```

### CMake
```bash
cmake -B build && cmake --build build
```

---

## Usage

```bash
# General summary (Header + Segments + Sections)
./elf-parser <file>

# Dump everything (mirrors readelf -a)
./elf-parser -a <file>

# View symbol table
./elf-parser -s <file>

# Hex dump a specific section
./elf-parser -x .text <file>
```

### CLI Options

| Flag | Description |
|---|---|
| `-h` | Display the ELF header |
| `-l` | Display program segment headers |
| `-S` | Display section headers |
| `-s` | Display symbol tables |
| `-r` | Display relocation entries |
| `-d` | Display dynamic section |
| `-x <sec>` | Hex & ASCII dump of a section |
| `-a` | Show all information |
| `--no-color` | Disable ANSI terminal colors |
