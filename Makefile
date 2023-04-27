CC=gcc-12

# Core code
CORE_SOURCES=$(wildcard core/src/*.c core/src/*/*c)
CORE_OBJECTS=$(CORE_SOURCES:.c=.o)
CORE_HEADERS=core/headers
CORE_EXECUTABLE=./rede-core


# CLI code
CLI_SOURCES=$(wildcard cli/src/*.c cli/src/*/*.c)
CLI_OBJECTS=$(CLI_SOURCES:.c=.o)
CLI_HEADERS=cli/headers
CLI_EXECUTABLE=./rede


# Lib names
RUNTIME_LIB_NAME=RedeRuntime.h
COMPILER_LIB_NAME=RedeCompiler.h
STD_LIB_NAME=RedeSTD.h
GENERAL_LIB_NAME=Rede.h


.PHONY: core
cli: $(CLI_EXECUTABLE)
	$(CLI_EXECUTABLE)

build-cli: $(CLI_EXECUTABLE)
	echo "Done"

$(CLI_EXECUTABLE): $(CLI_OBJECTS)
	$(CC) $(CFLAGS) -o $(CLI_EXECUTABLE) $^

$(CLI_OBJECTS): %.o: %.c $(GENERAL_LIB_NAME)
	$(CC) $(CFLAGS) -c -o $@ -I$(CLI_HEADERS) -I./ -Wall -Wextra -std=c99 -pedantic $<


.PHONY: core
core: $(CORE_EXECUTABLE)
	$(CORE_EXECUTABLE)

$(CORE_EXECUTABLE): $(CORE_OBJECTS)
	$(CC) $(CFLAGS) -o $(CORE_EXECUTABLE) $^

$(CORE_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ -I$(CORE_HEADERS) -Wall -Wextra -std=c99 -pedantic $<


# Builds runtime STB-like lib
$(RUNTIME_LIB_NAME): core/headers/RedeRuntime.h core/headers/RedeByteCodes.h core/src/RedeRuntime.c
	cat core/headers/RedeRuntime.h > $(RUNTIME_LIB_NAME)
	tail -n +2 core/headers/RedeRuntimeUtils.h >> $(RUNTIME_LIB_NAME)
	echo "\n#if defined(REDE_RUNTIME_IMPLEMENTATION)" >> $(RUNTIME_LIB_NAME)
	cat core/headers/RedeByteCodes.h >> $(RUNTIME_LIB_NAME)
	tail -n +2 core/src/RedeRuntimeUtils.c >> $(RUNTIME_LIB_NAME)
	tail -n +3 core/src/RedeRuntime.c >> $(RUNTIME_LIB_NAME)
	echo "\n#endif // REDE_RUNTIME_IMPLEMENTATION" >> $(RUNTIME_LIB_NAME)



# Builds compiler STB-like lib
$(COMPILER_LIB_NAME): core/headers/RedeCompiler.h core/headers/RedeSourceIterator.h core/headers/logs.h core/headers/RedeByteCodes.h core/src/RedeCompiler.c core/src/RedeSourceIterator.c
	cat core/headers/RedeCompiler.h > $(COMPILER_LIB_NAME)
	echo "\n#if defined(REDE_COMPILER_IMPLEMENTATION)" >> $(COMPILER_LIB_NAME)
	cat core/headers/RedeSourceIterator.h >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	tail -n +3 core/src/RedeSourceIterator.c >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	cat core/headers/logs.h >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	cat core/headers/RedeByteCodes.h >> $(COMPILER_LIB_NAME)
	echo "\n" >> $(COMPILER_LIB_NAME)
	tail -n +5 core/src/RedeCompiler.c >> $(COMPILER_LIB_NAME)
	echo "\n#endif // REDE_COMPILER_IMPLEMENTATION" >> $(COMPILER_LIB_NAME)



# Build STB-like standard library
$(STD_LIB_NAME): core/headers/RedeStd.h core/src/RedeStd.c
	echo "#if !defined(REDE_RUNTIME_PATH)" > $(STD_LIB_NAME)
	echo "#define REDE_RUNTIME_PATH \"RedeRuntime.h\"" >> $(STD_LIB_NAME)
	echo "#endif // REDE_RUNTIME_PATH\n" >> $(STD_LIB_NAME)
	echo "#include REDE_RUNTIME_PATH" >> $(STD_LIB_NAME)
	tail -n +3 core/headers/RedeStd.h >> $(STD_LIB_NAME)
	echo "\n#if defined(REDE_STD_IMPLEMENTATION)" >> $(STD_LIB_NAME)
	tail -n +2 core/src/RedeStd.c >> $(STD_LIB_NAME)
	echo "\n#endif // REDE_STD_IMPLEMENTATION" >> $(STD_LIB_NAME)



# Builds general lib
$(GENERAL_LIB_NAME): $(RUNTIME_LIB_NAME) $(COMPILER_LIB_NAME) $(STD_LIB_NAME)
	echo "#if defined(REDE_IMPLEMENTATION)" > $(GENERAL_LIB_NAME)
	echo "#define REDE_COMPILER_IMPLEMENTATION" >> $(GENERAL_LIB_NAME)
	echo "#define REDE_RUNTIME_IMPLEMENTATION" >> $(GENERAL_LIB_NAME)
	echo "#define REDE_STD_IMPLEMENTATION" >> $(GENERAL_LIB_NAME)
	echo "#endif // REDE_IMPLEMENTATION" >> $(GENERAL_LIB_NAME)
	cat $(RUNTIME_LIB_NAME) >> $(GENERAL_LIB_NAME)
	cat $(COMPILER_LIB_NAME) >> $(GENERAL_LIB_NAME)
	tail -n +7 $(STD_LIB_NAME) >> $(GENERAL_LIB_NAME)



.PHONY: libs
# Builds libs
libs: $(RUNTIME_LIB_NAME) $(COMPILER_LIB_NAME) $(STD_LIB_NAME) $(GENERAL_LIB_NAME)
	echo "\nDone"


.PHONY: tests
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



.PHONY: clean
clean:
	rm -f $(CORE_OBJECTS) $(CORE_EXECUTABLE) $(CLI_OBJECTS) $(CLI_EXECUTABLE)


.PHONY: clean-all
clean-all: clean
	rm -f $(RUNTIME_LIB_NAME) $(COMPILER_LIB_NAME) $(STD_LIB_NAME) $(GENERAL_LIB_NAME)