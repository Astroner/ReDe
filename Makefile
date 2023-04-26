CC=gcc-12
SOURCES=$(wildcard src/*.c src/*/*c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=headers

EXECUTABLE=./rede

RUNTIME_LIB_NAME=RedeRuntime.h
COMPILER_LIB_NAME=RedeCompiler.h
STD_LIB_NAME=RedeSTD.h
GENERAL_LIB_NAME=Rede.h

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

# Build standard library
std: headers/RedeStd.h src/RedeStd.c
	echo "#if !defined(REDE_RUNTIME_PATH)" > $(STD_LIB_NAME)
	echo "#define REDE_RUNTIME_PATH \"RedeRuntime.h\"" >> $(STD_LIB_NAME)
	echo "#endif // REDE_RUNTIME_PATH\n" >> $(STD_LIB_NAME)
	echo "#include REDE_RUNTIME_PATH" >> $(STD_LIB_NAME)
	tail -n +3 headers/RedeStd.h >> $(STD_LIB_NAME)
	echo "\n#if defined(REDE_STD_IMPLEMENTATION)" >> $(STD_LIB_NAME)
	tail -n +2 src/RedeStd.c >> $(STD_LIB_NAME)
	echo "\n#endif // REDE_STD_IMPLEMENTATION" >> $(STD_LIB_NAME)

# Builds general lib
general: runtime compiler std
	cat $(RUNTIME_LIB_NAME) > $(GENERAL_LIB_NAME)
	cat $(COMPILER_LIB_NAME) >> $(GENERAL_LIB_NAME)
	tail -n +7 $(STD_LIB_NAME) >> $(GENERAL_LIB_NAME)


# Builds libs
libs: runtime compiler std general
	echo "\nDone"


tests: libs
	$(CC) -o tests/runtime.test tests/runtime.c
	$(CC) -o tests/compiler.test tests/compiler.c
	$(CC) -o tests/std.test tests/std.c
	$(CC) -o tests/general.test tests/general.c

	tests/runtime.test
	tests/compiler.test
	tests/std.test
	tests/general.test

	rm -f tests/runtime.test tests/compiler.test tests/std.test tests/general.test


clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

clean-all: clean
	rm -f $(RUNTIME_LIB_NAME) $(COMPILER_LIB_NAME)