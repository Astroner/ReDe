#include "RedeCompiler.h"

#include <stdio.h>
#include <stdlib.h>

typedef enum RedeSourceIteratorType {
    RedeByteIteratorTypeString
} RedeSourceIteratorType;

typedef struct RedeSourceIterator {
    size_t index;
    RedeSourceIteratorType type;
    union {
        char* string;
    } data;
} RedeSourceIterator;

static int initIterator(RedeSource* src, RedeSourceIterator* iterator) {
    iterator->index = -1;
    switch(src->type) {
        case RedeSourceTypeString:
            iterator->type = RedeByteIteratorTypeString;
            iterator->data.string = src->data.string;
            break;

        case RedeSourceTypeFile:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
            break;
    }

    return 0;
}

static char nextChar(RedeSourceIterator* iterator) {
    iterator->index++;
    switch(iterator->type) {
        case RedeByteIteratorTypeString: {
            return iterator->data.string[iterator->index];
        }
        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}

static int parseAssign(RedeSourceIterator* iterator, CompilationMemory* memory, size_t identifierStart, size_t identifierLength) {
    
    return 0;
}

int Rede_compile(RedeSource* src, CompilationMemory* memory) {
    RedeSourceIterator iterator;
    initIterator(src, &iterator);

    size_t identifierStart = 0;
    size_t identifierLength = 0;

    int searchForOperation = 0;

    char ch;
    while((ch = nextChar(&iterator))) {
        if(searchForOperation) {
            if(ch == '=') {
                parseAssign(&iterator, memory, identifierStart, identifierLength);
            } else if(ch == '(') {

            } else if(ch == ' ') {

            } else {
                printf("Unexpected char '%c'(%d) while searching for '=' or '('\n", ch, ch);
                return -1;
            }
        } else {
            if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                identifierLength++;
            } else {
                if(identifierLength == 0) {
                    printf("Unexpected char '%c'(%d) while searching for identifier\n", ch, ch);
                    return -1;
                }
                searchForOperation = 1;
            }
        }
    }

    return 0;
}