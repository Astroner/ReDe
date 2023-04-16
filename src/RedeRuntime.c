#include "Rede.h"

#include "RedeByteCodes.h"

#include <stdlib.h>

typedef enum RedeByteIteratorType {
    RedeByteIteratorTypeBuffer
} RedeByteIteratorType;

typedef struct RedeByteIterator {
    RedeByteIteratorType type;
    union {
        struct {
            unsigned char* cursor;
        } buffer;
    } data;
} RedeByteIterator;

static int initIterator(RedeByteCode* src, RedeByteIterator* iterator) {
    switch(src->type) {
        case RedeByteCodeTypeBuffer:
            iterator->type = RedeByteIteratorTypeBuffer;
            iterator->data.buffer.cursor = src->data.buffer.buffer;
            break;

        case RedeByteCodeTypeFile:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
            break;
    }

    return 0;
}

static unsigned char nextByte(RedeByteIterator* iterator) {
    switch(iterator->type) {
        case RedeByteIteratorTypeBuffer: {
            unsigned char byte = iterator->data.buffer.cursor[0];
            iterator->data.buffer.cursor++;
            return byte;
        }
        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}

int copyToStringBuffer(RedeByteIterator* bytes, RedeRuntimeMemory* memory, RedeVariable* result) {
    size_t stringLength = (size_t)nextByte(bytes);

    if(memory->stringBufferLength < stringLength + 1) {
        return -1;
    }

    if(memory->stringBufferLength - memory->stringBufferActualLength < stringLength + 1) {
        memory->stringBufferActualLength = 0;
    }

    char* start = memory->stringBuffer + memory->stringBufferActualLength;

    for(unsigned int i = 0; i < stringLength; i++) {
        memory->stringBuffer[memory->stringBufferActualLength] = nextByte(bytes);
        memory->stringBufferActualLength++;
    }
    memory->stringBuffer[memory->stringBufferActualLength] = '\0';
    memory->stringBufferActualLength++;

    result->type = RedeVariableTypeString;
    result->data.string.string = start;
    result->data.string.length = stringLength + 1;

    return 0;
}


typedef union BytesToFloat {
    float number;
    char bytes[4];
} BytesToFloat;

static int setVariable(
    RedeByteIterator* bytes, 
    RedeRuntimeMemory* memory,
    RedeVariable* result
) {
    unsigned char type = nextByte(bytes);
    switch(type) {
        case REDE_TYPE_NUMBER: {
            BytesToFloat translator;
            translator.bytes[0] = nextByte(bytes);
            translator.bytes[1] = nextByte(bytes);
            translator.bytes[2] = nextByte(bytes);
            translator.bytes[3] = nextByte(bytes);
            result->type = RedeVariableTypeNumber;
            result->data.number = translator.number;
            break;
        }
        case REDE_TYPE_STRING:
            copyToStringBuffer(bytes, memory, result);
            break;
        case REDE_TYPE_VAR: {
            unsigned char index = nextByte(bytes);
            if(index >= memory->variablesBufferSize) {
                return -1;
            }
            RedeVariable src = memory->variablesBuffer[index];
            result->type = src.type;
            result->data = src.data;
            break;
        }
        case REDE_TYPE_STACK: {
            if(memory->stackActualSize == 0) return -2;
            memory->stackActualSize -= 1;
            RedeVariable src = memory->stack[memory->stackActualSize];
            result->type = src.type;
            result->data = src.data;
            break;
        }
    }
    result->busy = 1;

    return 0;
}

static int assignVariable(
    RedeByteIterator* bytes, 
    RedeRuntimeMemory* memory
) {
    unsigned char index = nextByte(bytes);
    if(index >= memory->variablesBufferSize) {
        return -1;
    }
    RedeVariable* variable = memory->variablesBuffer + index;

    int status = setVariable(bytes, memory, variable);

    if(status < 0) {
        return status - 1;
    }

    return 0;
}

static int putOnStack(
    RedeByteIterator* bytes, 
    RedeRuntimeMemory* memory
) {
    RedeVariable* variable = memory->stack + memory->stackActualSize;

    if(memory->stackActualSize + 1 >= memory->stackSize) {
        return -1;
    }

    int status = setVariable(bytes, memory, variable);
    
    if(status < 0) {
        return status - 1;
    }

    memory->stackActualSize += 1;

    return 0;
} 

static int functionCall(
    RedeByteIterator* bytes, 
    RedeRuntimeMemory* memory,
    int (*funcCall)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
) {
    RedeVariable name;
    copyToStringBuffer(bytes, memory, &name);

    unsigned int argumentsNumber = nextByte(bytes);

    if(memory->stackActualSize < argumentsNumber) {
        printf(
            "Not enough variables on the stack to call function '%s'. Actual length: %zu, Required: %d\n", 
            name.data.string.string, memory->stackActualSize, argumentsNumber
        );
        return -1;
    }
    memory->stackActualSize -= argumentsNumber;
    RedeVariable* argumentsStart = memory->stack + memory->stackActualSize;

    RedeFunctionArgs args = {
        .length = argumentsNumber,
        .values = argumentsStart,
    };

    RedeVariable result;

    int status = funcCall(name.data.string.string, name.data.string.length, &args, &result, sharedData);

    if(status < 0) {
        return status - 1;
    }

    argumentsStart->type = result.type;
    argumentsStart->data = result.data;
    memory->stackActualSize++;

    return 0;
} 

int Rede_execute(
    RedeByteCode* program, 
    RedeRuntimeMemory* memory,
    int (*funcCall)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
) {
    RedeByteIterator iterator;
    initIterator(program, &iterator);
    
    unsigned char code;
    int status;
    while((code = nextByte(&iterator)) != REDE_CODE_END) {
        switch(code) {
            case REDE_CODE_ASSIGN:
                status = assignVariable(&iterator, memory);
                break;
            case REDE_CODE_STACK_PUSH:
                status = putOnStack(&iterator, memory);
                break;
            case REDE_CODE_CALL:
                status = functionCall(&iterator, memory, funcCall, sharedData);
                break;
            case REDE_CODE_STACK_CLEAR:
                memory->stackActualSize = 0;
                break;
            default:
                printf("Unknown statement\n");
        }
        if(status < 0) {
            printf("Something went wrong\n");
            return -1;
        }
    }

    return 0;
}
