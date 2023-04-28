#if !defined(REDE_COMPILER_H)
#define REDE_COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define Rede_createStringSource(name, code)\
    RedeSource name##__data = {\
        .type = RedeSourceTypeString,\
        .data = {\
            .string = code\
        }\
    };\
    RedeSource* name = &name##__data;

#define Rede_createFileSource(name, pathToFile)\
    RedeSource name##__data = {\
        .type = RedeSourceTypeFile,\
        .data = {\
            .path = pathToFile\
        }\
    };\
    RedeSource* name = &name##__data;



typedef enum RedeDestType {
    RedeDestTypeBuffer,
    RedeDestTypeFile
} RedeDestType;

typedef struct RedeDest {
    RedeDestType type;
    union {
        struct {
            unsigned char* buffer;
            size_t length;
            size_t maxLength;
        } buffer;
        struct {
            char* path;
            FILE* fp;
        } file;
    } data;
} RedeDest;

#define Rede_createBufferDest(name, bufferLength)\
    unsigned char name##__buffer[bufferLength];\
    memset(name##__buffer, 0, sizeof(name##__buffer));\
    RedeDest name##__data = {\
        .type = RedeDestTypeBuffer,\
        .data = {\
            .buffer = {\
                .buffer = name##__buffer,\
                .length = 0,\
                .maxLength = bufferLength,\
            }\
        }\
    };\
    RedeDest* name = &name##__data;\


#define Rede_createFileDest(name, filePath)\
    RedeDest name##__data = {\
        .type = RedeDestTypeFile,\
        .data = {\
            .file = {\
                .path = filePath,\
            }\
        }\
    };\
    RedeDest* name = &name##__data;\



typedef struct RedeVariableName {
    int isBusy;
    unsigned char index;
    size_t start;
    size_t length;
} RedeVariableName;

typedef struct RedeCompilationMemory {
    struct {
        unsigned char nextIndex;
        RedeVariableName* buffer;
        size_t bufferSize;
    } variables;
} RedeCompilationMemory;

#define Rede_createCompilationMemory(name, variablesBufferSize)\
    RedeVariableName name##__names[256];\
    memset(name##__names, 0, sizeof(name##__names));\
    RedeCompilationMemory name##__data = {\
        .variables = {\
            .buffer = name##__names,\
            .bufferSize = variablesBufferSize,\
            .nextIndex = 0,\
        }\
    };\
    RedeCompilationMemory* name = &name##__data;


int Rede_compile(RedeSource* src, RedeCompilationMemory* memory, RedeDest* dist);

#endif // REDE_COMPILER_H
