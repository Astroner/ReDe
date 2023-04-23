#if !defined(REDE_COMPILER_H)
#define REDE_COMPILER_H

#include <stdio.h>
#include <stdlib.h>

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


#define Rede_createStringSource(name, code)\
    RedeSource name##__data = {\
        .type = RedeSourceTypeString,\
        .data = {\
            .string = code\
        }\
    };\
    RedeSource* name = &name##__data;



#define Rede_createCompilationMemory(name, programBufferSize, variablesBufferSize)\
    unsigned char name##__buffer[programBufferSize];\
    memset(name##__buffer, 0, sizeof(name##__buffer));\
    RedeVariableName name##__names[256];\
    memset(name##__names, 0, sizeof(name##__names));\
    RedeCompilationMemory name##__data = {\
        .buffer = name##__buffer,\
        .bufferLength = programBufferSize,\
        .bufferActualLength = 0,\
        .variables = {\
            .buffer = name##__names,\
            .bufferSize = variablesBufferSize,\
            .nextIndex = 0,\
        }\
    };\
    RedeCompilationMemory* name = &name##__data;

int Rede_compile(RedeSource* src, RedeCompilationMemory* memory);

#endif // REDE_COMPILER_H
