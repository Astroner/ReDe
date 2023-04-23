CC=gcc-12
SOURCES=$(wildcard src/*.c src/*/*c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=headers

EXECUTABLE=./rede

RUNTIME_LIB_NAME=RedeRuntime.h

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
	echo "#if defined(REDE_RUNTIME_IMPLEMENTATION)" >> $(RUNTIME_LIB_NAME)
	cat headers/RedeByteCodes.h >> $(RUNTIME_LIB_NAME)
	tail -n +2 src/RedeRuntimeUtils.c >> $(RUNTIME_LIB_NAME)
	tail -n +3 src/RedeRuntime.c >> $(RUNTIME_LIB_NAME)
	echo "\n#endif // REDE_RUNTIME_IMPLEMENTATION" >> $(RUNTIME_LIB_NAME)

# Builds libs
libs: runtime
	echo "\nDone"

tests: runtime
	$(CC) -o tests/runtime.test tests/runtime.c
	tests/runtime.test
	rm -f tests/runtime.test

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

clean-all: clean
	rm -f $(RUNTIME_LIB_NAME)