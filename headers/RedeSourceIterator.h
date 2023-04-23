#if !defined(REDE_SOURCE_ITERATOR)
#define REDE_SOURCE_ITERATOR

typedef enum RedeSourceIteratorType {
    RedeSourceIteratorTypeString
} RedeSourceIteratorType;

typedef struct RedeSourceIterator {
    size_t index;
    int finished;
    RedeSourceIteratorType type;
    union {
        char* string;
    } data;
} RedeSourceIterator;

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator);
char RedeSourceIterator_nextChar(RedeSourceIterator* iterator);
char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index);
char RedeSourceIterator_current(RedeSourceIterator* iterator);

#endif // REDE_SOURCE_ITERATOR
