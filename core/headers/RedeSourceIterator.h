#include "RedeCompiler.h"

#include <stdio.h>

#if !defined(REDE_SOURCE_ITERATOR)
#define REDE_SOURCE_ITERATOR

typedef enum RedeSourceIteratorType {
    RedeSourceIteratorTypeString,
    RedeSourceIteratorTypeFile
} RedeSourceIteratorType;

typedef struct RedeSourceIterator {
    size_t index;
    int finished;
    char current;
    RedeSourceIteratorType type;
    union {
        char* string;
        struct {
            FILE* fp;
        } file;
    } data;
} RedeSourceIterator;

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator);
void RedeSourceIterator_destroy(RedeSourceIterator* iterator);

char RedeSourceIterator_nextChar(RedeSourceIterator* iterator);
char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index);
char RedeSourceIterator_current(RedeSourceIterator* iterator);
void RedeSourceIterator_moveCursorBack(RedeSourceIterator* iterator, size_t shift);

#endif // REDE_SOURCE_ITERATOR
