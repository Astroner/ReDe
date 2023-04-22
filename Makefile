CC=gcc-12
SOURCES=$(wildcard src/*.c src/*/*c)
OBJECTS=$(SOURCES:.c=.o)
HEADERS=headers

EXECUTABLE=./rede

all: build
	$(EXECUTABLE)

build: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^

.c.o:
	$(CC) $(CFLAGS) -c -o $@ -I$(HEADERS) -Wall -Wextra -std=c99 -pedantic $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)