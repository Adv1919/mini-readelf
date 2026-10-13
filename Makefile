# =============================================================================
# Makefile  —  Quick build for Linux / WSL / macOS
# Usage:
#   make              → build release binary
#   make debug        → build with debug symbols
#   make clean        → remove build artefacts
#   make test         → build and run against /bin/ls (Linux only)
# =============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Isrc
RELEASE  := -O2 -DNDEBUG
DEBUG    := -O0 -g3 -fsanitize=address,undefined

TARGET   := elf-parser
SOURCES  := src/elf_parser.cpp src/main.cpp
HEADERS  := src/elf_types.h src/elf_parser.h

.PHONY: all debug clean test

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(RELEASE) $(SOURCES) -o $@
	@echo "Built: ./$@"

debug: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(DEBUG) $(SOURCES) -o $(TARGET)-debug
	@echo "Built: ./$(TARGET)-debug"

clean:
	rm -f $(TARGET) $(TARGET)-debug

test: $(TARGET)
	@echo "─── Testing against /bin/ls ───"
	./$(TARGET) -a /bin/ls | head -80
	@echo ""
	@echo "─── Hex dump of .interp ───"
	./$(TARGET) -x .interp /bin/ls
	@echo ""
	@echo "─── Relocatable object test ───"
	echo 'int add(int a,int b){return a+b;} int main(){return add(1,2);}' > /tmp/_test.c
	gcc -c /tmp/_test.c -o /tmp/_test.o
	./$(TARGET) -a /tmp/_test.o
	rm -f /tmp/_test.c /tmp/_test.o
