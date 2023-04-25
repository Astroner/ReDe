CC=gcc-12
SOURCES=$(wildcard src/*.c src/*/*c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=headers

EXECUTABLE=./rede

RUNTIME_LIB_NAME=RedeRuntime.h
COMPILER_LIB_NAME=RedeCompiler.h

all: build
	$(EXECUTABLE)

build: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^

.c.o:
	$(CC) $(CFLAGS) -c -o $@ -I$(HEADERS) -Wall -Wextra -std=c99 -pedantic $<

# Builds runtime STB-like lib
runtime: headers/RedeRuntime.h headers/RedeByteCodes.h src/RedeRuntime.c
	cat headers/RedeRuntime.h > $(RUNTIME_LIB_NAME)
	tail -n +2 headers/RedeRuntimeUtils.h >> $(RUNTIME_LIB_NAME)
	echo "\n#if defined(REDE_RUNTIME_IMPLEMENTATION)" >> $(RUNTIME_LIB_NAME)
	cat headers/RedeByteCodes.h >> $(RUNTIME_LIB_NAME)
	tail -n +2 src/RedeRuntimeUtils.c >> $(RUNTIME_LIB_NAME)
	tail -n +3 src/RedeRuntime.c >> $(RUNTIME_LIB_NAME)
	echo "\n#endif // REDE_RUNTIME_IMPLEMENTATION" >> $(RUNTIME_LIB_NAME)

# Builds compiler STB-like lib
compiler: headers/RedeCompiler.h headers/RedeSourceIterator.h headers/logs.h headers/RedeByteCodes.h src/RedeCompiler.c src/RedeSourceIterator.c
	cat headers/RedeCompiler.h > $(COMPILER_LIB_NAME)
	echo "\n#if defined(REDE_COMPILER_IMPLEMENTATION)" >> $(COMPILER_LIB_NAME)
	cat headers/RedeSourceIterator.h >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	tail -n +3 src/RedeSourceIterator.c >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	cat headers/logs.h >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	cat headers/RedeByteCodes.h >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	tail -n +5 src/RedeCompiler.c >> $(COMPILER_LIB_NAME)
	echo "\n#endif // REDE_COMPILER_IMPLEMENTATION" >> $(COMPILER_LIB_NAME)


# Builds libs
libs: runtime compiler
	echo "\nDone"



tests: runtime compiler
	$(CC) -o tests/runtime.test tests/runtime.c
	$(CC) -o tests/compiler.test tests/compiler.c

	tests/runtime.test
	tests/compiler.test

	rm -f tests/runtime.test tests/compiler.test



clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

clean-all: clean
	rm -f $(RUNTIME_LIB_NAME) $(COMPILER_LIB_NAME)