#if !defined(REDE_RUNTIME_H)
#define REDE_RUNTIME_H

#include <stdio.h>
#include <string.h>


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
    RedeVariableTypeBoolean,
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
        int boolean;
    } data;
} RedeVariable;

typedef struct RedeFunctionArgs {
    RedeVariable* values;
    int length;
} RedeFunctionArgs;

typedef struct RedeRuntimeMemory {
    RedeVariable* variablesBuffer;
    size_t variablesBufferSize;
    RedeVariable* stack;
    size_t stackSize;
    size_t stackActualSize;
    char* stringBuffer;
    size_t stringBufferLength;
    size_t stringBufferActualLength;
} RedeRuntimeMemory;

#define Rede_createByteCodeFromBuffer(name, bytesBuffer)\
    RedeByteCode name##__data = {\
        .type = RedeByteCodeTypeBuffer,\
        .data = {\
            .buffer = {\
                .buffer = (bytesBuffer)\
            }\
        }\
    };\
    RedeByteCode* name = &name##__data;

#define Rede_createByteCodeFromFile(name, filePath)\
    RedeByteCode name##__data = {\
        .type = RedeByteCodeTypeFile,\
        .data = {\
            .file = {\
                .path = (filePath)\
            }\
        }\
    };\
    RedeByteCode* name = &name##__data;


#define Rede_createByteCode(name, ...)\
    unsigned char name##__buffer[] = { __VA_ARGS__ };\
    RedeByteCode name##__data = {\
        .type = RedeByteCodeTypeBuffer,\
        .data = {\
            .buffer = {\
                .buffer = name##__buffer\
            }\
        }\
    };\
    RedeByteCode* name = &name##__data;


#define Rede_createRuntimeMemory(name, stackSizeA, variablesBufferSizeA, stringBufferSizeA)\
    RedeVariable name##__stack[stackSizeA];\
    memset(name##__stack, 0, sizeof(name##__stack));\
    RedeVariable name##__variables[variablesBufferSizeA];\
    memset(name##__variables, 0, sizeof(name##__variables));\
    char name##__strings[stringBufferSizeA];\
    memset(name##__strings, 0, stringBufferSizeA);\
    RedeRuntimeMemory name##__data = {\
        .stack = name##__stack,\
        .stackActualSize = 0,\
        .stackSize = stackSizeA,\
        .stringBuffer = name##__strings,\
        .stringBufferActualLength = 0,\
        .stringBufferLength = stringBufferSizeA,\
        .variablesBuffer = name##__variables,\
        .variablesBufferSize = variablesBufferSizeA,\
    };\
    RedeRuntimeMemory* name = &name##__data;

int Rede_execute(
    RedeByteCode* program, 
    RedeRuntimeMemory* memory,
    int (*)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
);

#endif // REDE_RUNTIME_H
