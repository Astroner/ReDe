#if !defined(REDE_H)
#define REDE_H

#include <stdio.h>


typedef enum RedeByteCodeType {
    RedeByteCodeTypeFile,
    RedeByteCodeTypeBuffer,
} RedeByteCodeType;

typedef struct RedeByteCode {
    RedeByteCodeType type;
    union {
        struct {
            char* path;
        } file;
        struct {
            unsigned char* buffer;
        } buffer;
    } data;
} RedeByteCode;

typedef enum RedeVariableType {
    RedeVariableTypeNumber,
    RedeVariableTypeString,
} RedeVariableType;

typedef struct RedeVariable {
    int busy;
    RedeVariableType type;
    union {
        float number;
        struct {
            char* string;
            int length;
        } string;
    } data;
} RedeVariable;

typedef struct RedeFunctionArgs {
    RedeVariable* values;
    int length;
} RedeFunctionArgs;

typedef struct RedeRuntimeMemory {
    RedeVariable* variablesBuffer;
    unsigned char variablesBufferSize;
    RedeVariable* stack;
    size_t stackSize;
    size_t stackActualSize;
    char* stringBuffer;
    size_t stringBufferLength;
    size_t stringBufferActualLength;
} RedeRuntimeMemory;

int Rede_execute(
    RedeByteCode* program, 
    RedeRuntimeMemory* memory,
    int (*)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
);

#endif // REDE_H
