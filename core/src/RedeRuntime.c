#include "RedeRuntime.h"
#include "RedeByteCodes.h"
#include "RedeByteIterator.h"

#include <stdlib.h>


int copyToStringBuffer(RedeByteIterator* bytes, RedeRuntimeMemory* memory, RedeVariable* result) {
    size_t stringLength = (size_t)RedeByteIterator_nextByte(bytes);

    if(memory->stringBufferLength < stringLength + 1) {
        return -1;
    }

    if(memory->stringBufferLength - memory->stringBufferActualLength < stringLength + 1) {
        memory->stringBufferActualLength = 0;
    }

    char* start = memory->stringBuffer + memory->stringBufferActualLength;

    for(unsigned int i = 0; i < stringLength; i++) {
        memory->stringBuffer[memory->stringBufferActualLength] = RedeByteIterator_nextByte(bytes);
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
    unsigned char type = RedeByteIterator_nextByte(bytes);
    switch(type) {
        case REDE_TYPE_NUMBER: {
            BytesToFloat translator;
            translator.bytes[0] = RedeByteIterator_nextByte(bytes);
            translator.bytes[1] = RedeByteIterator_nextByte(bytes);
            translator.bytes[2] = RedeByteIterator_nextByte(bytes);
            translator.bytes[3] = RedeByteIterator_nextByte(bytes);
            result->type = RedeVariableTypeNumber;
            result->data.number = translator.number;
            break;
        }
        case REDE_TYPE_STRING:
            copyToStringBuffer(bytes, memory, result);
            break;
        case REDE_TYPE_BOOL:
            result->type = RedeVariableTypeBoolean;
            result->data.boolean = RedeByteIterator_nextByte(bytes) == 0 ? 0 : 1;
            break;
        case REDE_TYPE_VAR: {
            unsigned char index = RedeByteIterator_nextByte(bytes);
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
    unsigned char index = RedeByteIterator_nextByte(bytes);
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

    unsigned int argumentsNumber = RedeByteIterator_nextByte(bytes);

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
        if(status == -2) {
            printf("Function '%s' was not provided\n", name.data.string.string);
        }
        return status - 1;
    }
    memory->stringBufferActualLength -= name.data.string.length;
    argumentsStart->type = result.type;
    argumentsStart->data = result.data;
    memory->stackActualSize++;

    return 0;
}
 
static int parseDestination(RedeByteIterator* bytes) {
    int direction = RedeByteIterator_nextByte(bytes);

    int result = 0;
    
    unsigned char* bts = (unsigned char*)&result;
    bts[0] = RedeByteIterator_nextByte(bytes);
    bts[1] = RedeByteIterator_nextByte(bytes);

    if(direction != REDE_DIRECTION_FORWARD) {
        result *= -1;
        result -= 2;
    }

    return result;
}

static int reduceToBoolean(RedeVariable* var) {
    switch(var->type) {
        case RedeVariableTypeString:
            // Counting with NULL-terminator
            return var->data.string.length > 1;
        case RedeVariableTypeNumber:
            return var->data.number != 0;
        case RedeVariableTypeBoolean:
            return var->data.boolean;
    }
    return 0;
}

static int conditionalJump(RedeByteIterator* iterator, RedeRuntimeMemory* memory, int reversedCondition) {
    RedeVariable condition;
    setVariable(iterator, memory, &condition);
    int value = reduceToBoolean(&condition);
    int shift = parseDestination(iterator);
    if(value && !reversedCondition) {
        RedeByteIterator_moveCursor(iterator, shift);
    } else if(!value && reversedCondition) {
        RedeByteIterator_moveCursor(iterator, shift);
    }

    return 0;
}

#define EXIT_EXECUTION(code)\
    executionCode = code;\
    goto exit_execution;\

int Rede_execute(
    RedeByteCode* program, 
    RedeRuntimeMemory* memory,
    int (*funcCall)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
) {
    int executionCode = 0;

    RedeByteIterator iterator;
    if(RedeByteIterator_init(program, &iterator) < 0) {
        return -1;
    }
    
    unsigned char code;
    int status;
    while((code = RedeByteIterator_nextByte(&iterator)) != REDE_CODE_END) {
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

            case REDE_CODE_JUMP: {
                int shift = parseDestination(&iterator);
                RedeByteIterator_moveCursor(&iterator, shift);
                status = 0;
                break;
            }

            case REDE_CODE_JUMP_IF: 
                status = conditionalJump(&iterator, memory, 0);
                break;

            case REDE_CODE_JUMP_IF_NOT: 
                status = conditionalJump(&iterator, memory, 1);
                break;

            case REDE_CODE_STACK_CLEAR:
                memory->stackActualSize = 0;
                status = 0;
                break;
            
            case REDE_CODE_NOP:
                status = 0;
                break;

            default:
                printf("Unknown statement %d\n", code);
                EXIT_EXECUTION(-1);
        }
        if(status < 0) {
            printf("Something went wrong with instruction 0x%X\n", code);
            EXIT_EXECUTION(-1);
        }
    }

exit_execution:
    RedeByteIterator_destroy(&iterator);
    return executionCode;
}
