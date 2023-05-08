#include "RedeRuntime.h"

#if !defined(REDE_BYTE_ITERATOR)
#define REDE_BYTE_ITERATOR

typedef enum RedeByteIteratorType {
    RedeByteIteratorTypeBuffer,
    RedeByteIteratorTypeFile
} RedeByteIteratorType;

typedef struct RedeByteIterator {
    RedeByteIteratorType type;
    union {
        struct {
            unsigned char* cursor;
        } buffer;
        struct {
            FILE* fp;
        } file;
    } data;
} RedeByteIterator;

int RedeByteIterator_init(RedeByteCode* src, RedeByteIterator* iterator);
void RedeByteIterator_destroy(RedeByteIterator* iterator);
unsigned char RedeByteIterator_nextByte(RedeByteIterator* iterator);
int RedeByteIterator_moveCursor(RedeByteIterator* iterator, int shift);


#endif // REDE_BYTE_ITERATOR
