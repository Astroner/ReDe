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

typedef struct CompilationMemory {
    unsigned char* buffer;
    size_t bufferLength;
    size_t bufferActualLength;
} CompilationMemory;

int Rede_compile(RedeSource* src, CompilationMemory* memory);

#endif // REDE_COMPILER_H
