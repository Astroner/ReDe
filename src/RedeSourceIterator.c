#include "RedeCompiler.h"
#include "RedeSourceIterator.h"

#include <stdio.h>
#include <stdlib.h>

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator) {
    iterator->index = -1;
    iterator->finished = 0;
    switch(src->type) {
        case RedeSourceTypeString:
            iterator->type = RedeSourceIteratorTypeString;
            iterator->data.string = src->data.string;
            break;

        case RedeSourceTypeFile:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
            break;
    }

    return 0;
}

char RedeSourceIterator_nextChar(RedeSourceIterator* iterator) {
    if(iterator->finished) return '\0';
    iterator->index++;
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: {
            char ch = iterator->data.string[iterator->index];
            if(!ch) iterator->finished = 1;
            return ch;
        }
        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}

char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index) {
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: 
            return iterator->data.string[index];

        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}

char RedeSourceIterator_current(RedeSourceIterator* iterator) {
    switch(iterator->type) {
        case RedeSourceIteratorTypeString:
            return iterator->data.string[iterator->index];
            
        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}