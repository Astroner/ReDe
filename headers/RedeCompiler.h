#if !defined(REDE_COMPILER_H)
#define REDE_COMPILER_H

#include <stdio.h>

typedef enum RedeSourceType {
    RedeSourceTypeFile,
    RedeSourceTypeString,
} RedeSourceType;

typedef struct RedeSource {
    RedeSourceType type;
    union {
        char* path;
        char* string;
    } data;
} RedeSource;

typedef struct RedeVariableName {
    int isBusy;
    unsigned char index;
    size_t start;
    size_t length;
} RedeVariableName;

typedef struct RedeCompilationMemory {
    unsigned char* buffer;
    size_t bufferLength;
    size_t bufferActualLength;
    struct {
        unsigned char nextIndex;
        RedeVariableName* buffer;
        size_t bufferSize;
    } variables;
} RedeCompilationMemory;

int Rede_compile(RedeSource* src, RedeCompilationMemory* memory);

#endif // REDE_COMPILER_H
