#include "RedeCompiler.h"
#include "RedeSourceIterator.h"



#include <stdio.h>
#include <stdlib.h>

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator) {
    iterator->index = -1;
    iterator->finished = 0;
    iterator->current = 0;
    switch(src->type) {
        case RedeSourceTypeString:
            iterator->type = RedeSourceIteratorTypeString;
            iterator->data.string = src->data.string;
            break;

        case RedeSourceTypeFile: {
            FILE* fp = fopen(src->data.path, "r");
            if(!fp) return -1;
            iterator->type = RedeSourceIteratorTypeFile;
            iterator->data.file.fp = fp;
            break;
        }
    }

    return 0;
}

void RedeSourceIterator_destroy(RedeSourceIterator* iterator) {
    if(iterator->type == RedeSourceIteratorTypeFile) {
        fclose(iterator->data.file.fp);
    }
}

char RedeSourceIterator_nextChar(RedeSourceIterator* iterator) {
    if(iterator->finished) return '\0';
    iterator->index++;
    switch(iterator->type) 
        case RedeSourceIteratorTypeString: {
            iterator->current = iterator->data.string[iterator->index];
            if(!iterator->current) iterator->finished = 1;

            break;

        case RedeSourceIteratorTypeFile:
            iterator->current = getc(iterator->data.file.fp);
            if(iterator->current == EOF) {
                iterator->finished = 1;
                iterator->current = '\0';
            }
            
            break;

        default:
            fprintf(stderr, "Unknown iterator type\n");
            exit(1);
    }
    return iterator->current;
}

char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index) {
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: 
            return iterator->data.string[index];

        case RedeSourceIteratorTypeFile: {
            size_t diff = iterator->index - index;
            fseek(iterator->data.file.fp, -diff - 1, SEEK_CUR);
            char ch = getc(iterator->data.file.fp);
            fseek(iterator->data.file.fp, diff, SEEK_CUR);
            
            return ch;
        }

        default:
            fprintf(stderr, "Unknown iterator type\n");
            exit(1);
    }
}

char RedeSourceIterator_current(RedeSourceIterator* iterator) {
    return iterator->current;
}

void RedeSourceIterator_moveCursorBack(RedeSourceIterator* iterator, size_t shift) {
    iterator->index -= shift;
    
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: 
            iterator->current = iterator->data.string[iterator->index];
            break;

        case RedeSourceIteratorTypeFile:
            fseek(iterator->data.file.fp, iterator->index, SEEK_SET);
            iterator->current = getc(iterator->data.file.fp);
            fseek(iterator->data.file.fp, -1, SEEK_CUR);
            break;

        default:
            fprintf(stderr, "Unknown iterator type\n");
            exit(1);
    }
}