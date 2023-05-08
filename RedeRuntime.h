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

#if !defined(REDE_RUNTIME_UTILS_H)
#define REDE_RUNTIME_UTILS_H

void Rede_printVariable(RedeVariable*);
void Rede_printlnVariable(RedeVariable*);
void Rede_setNumber(RedeVariable* variable, float number);
void Rede_setString(RedeVariable* variable, char* string, size_t length);
void Rede_setBoolean(RedeVariable* variable, int value);
void Rede_printMemory(RedeRuntimeMemory*);
void Rede_printBytecode(RedeByteCode* code);

#endif // REDE_RUNTIME_UTILS_H

#if defined(REDE_RUNTIME_IMPLEMENTATION)
#if !defined(REDE_BYTE_CODES)
#define REDE_BYTE_CODES

#define REDE_TYPE_NUMBER            0x00
#define REDE_TYPE_STRING            0x01
#define REDE_TYPE_VAR               0x02
#define REDE_TYPE_STACK             0x03
#define REDE_TYPE_BOOL              0x04

#define REDE_DIRECTION_FORWARD      0x00
#define REDE_DIRECTION_BACKWARD     0x01

#define REDE_CODE_ASSIGN            0x00
#define REDE_CODE_STACK_PUSH        0x01
#define REDE_CODE_CALL              0x02
#define REDE_CODE_STACK_CLEAR       0x03
#define REDE_CODE_JUMP              0x04
#define REDE_CODE_JUMP_IF           0x05
#define REDE_CODE_JUMP_IF_NOT       0x06

#define REDE_CODE_NOP               0xFE
#define REDE_CODE_END               0xFF

#endif // REDE_BYTE_CODES



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



#include <stdlib.h>

int RedeByteIterator_init(RedeByteCode* src, RedeByteIterator* iterator) {
    switch(src->type) {
        case RedeByteCodeTypeBuffer:
            iterator->type = RedeByteIteratorTypeBuffer;
            iterator->data.buffer.cursor = src->data.buffer.buffer;
            break;

        case RedeByteCodeTypeFile:
            iterator->type = RedeByteIteratorTypeFile;
            iterator->data.file.fp = fopen(src->data.file.path, "rb");
            if(!iterator->data.file.fp) return -1;
            break;
        
        default:
            return -1;
    }

    return 0;
}

void RedeByteIterator_destroy(RedeByteIterator* iterator) {
    if(iterator->type == RedeByteIteratorTypeFile) {
        fclose(iterator->data.file.fp);
    }
}

unsigned char RedeByteIterator_nextByte(RedeByteIterator* iterator) {
    switch(iterator->type) {
        case RedeByteIteratorTypeBuffer: {
            unsigned char byte = iterator->data.buffer.cursor[0];
            iterator->data.buffer.cursor++;
            return byte;
        }
        case RedeByteIteratorTypeFile: {
            int el = getc(iterator->data.file.fp);
            if(el == EOF || el < 0 || el > 255) {
                return REDE_CODE_END;
            }
            return (unsigned char) el;
        }
        default:
            fprintf(stderr, "Unknown iterator type\n");
            exit(1);
    }
}

int RedeByteIterator_moveCursor(RedeByteIterator* iterator, int shift) {
    switch(iterator->type) {
        case RedeByteIteratorTypeBuffer:
            iterator->data.buffer.cursor += shift;
            break;

        case RedeByteIteratorTypeFile:
            fseek(iterator->data.file.fp, shift, SEEK_CUR);
            break;

        default:
            return -1;
    }

    return 0;
}



void Rede_setNumber(RedeVariable* variable, float number) {
    variable->type = RedeVariableTypeNumber;
    variable->data.number = number;
}

void Rede_setString(RedeVariable* variable, char* string, size_t length) {
    variable->type = RedeVariableTypeString;
    variable->data.string.string = string;
    variable->data.string.length = length;
}

void Rede_setBoolean(RedeVariable* variable, int value) {
    variable->type = RedeVariableTypeBoolean;
    variable->data.boolean = value == 0 ? 0 : 1;
}

void Rede_printVariable(RedeVariable* variables) {
    switch(variables->type) {
        case RedeVariableTypeNumber:
            printf("%f", variables->data.number);
            break;
        case RedeVariableTypeString:
            printf("'%s'", variables->data.string.string);
            break;
        case RedeVariableTypeBoolean:
            printf(variables->data.boolean ? "true" : "false");
            break;
        default:
            printf("Unknown type\n");
    }
}

void Rede_printlnVariable(RedeVariable* variables) {
    Rede_printVariable(variables);
    printf("\n");
}

void Rede_printMemory(RedeRuntimeMemory* memory) {
    printf("Stack: (%zu/%zu)\n", memory->stackActualSize, memory->stackSize);
    for(size_t i = 0; i < memory->stackActualSize; i++) {
        printf("%zu) ", i);
        Rede_printlnVariable(memory->stack + i);
    }
    printf("\nVariables: (%zu)\n", memory->variablesBufferSize);
    for(size_t i = 0; i < memory->variablesBufferSize; i++) {
        if(memory->variablesBuffer[i].busy) {
            printf("%zu) ", i);
            Rede_printlnVariable(memory->variablesBuffer + i);
        }
    }
    printf("\nString Buffer: (%zu/%zu)\n", memory->stringBufferActualLength, memory->stringBufferLength);
    for(size_t i = 0; i < memory->stringBufferLength; i++) {
        if(i % 32 == 0 && i > 0) {
            printf("|\n|");
        } else if(i == 0) {
            printf("|");
        }
        if(i < memory->stringBufferActualLength) {
            if(memory->stringBuffer[i]) {
                printf("%c", memory->stringBuffer[i]);
            } else {
                printf(" ");
            }
        } else {
            printf(".");
        }
    }
    printf("|\n");
}




int Rede_printByteCode_type(RedeByteIterator* iterator) {
    int variableType = RedeByteIterator_nextByte(iterator);

    switch(variableType) {
        case REDE_TYPE_BOOL: 
            printf(RedeByteIterator_nextByte(iterator) > 0 ? "true" : "false");
            break;

        case REDE_TYPE_NUMBER: {
            float number = 0;
            unsigned char* bytes = (unsigned char*)&number;
            bytes[0] = RedeByteIterator_nextByte(iterator);
            bytes[1] = RedeByteIterator_nextByte(iterator);
            bytes[2] = RedeByteIterator_nextByte(iterator);
            bytes[3] = RedeByteIterator_nextByte(iterator);

            printf("%f", number);
            break;
        }

        case REDE_TYPE_STACK: 
            printf("Stack value");
            break;
        
        case REDE_TYPE_STRING: {
            printf("\'");
            int strLength = RedeByteIterator_nextByte(iterator);
            for(int i = 0; i < strLength; i++) {
                printf("%c", RedeByteIterator_nextByte(iterator));
            }
            printf("\'");
            break;
        }
        
        case REDE_TYPE_VAR:
            printf("Variable '%d'", RedeByteIterator_nextByte(iterator));

            break;
        
        default:
            printf("Unknown type %d\n", variableType);
            return -1;

    }

    return 0;
}

int Rede_printByteCode_assignment(RedeByteIterator* iterator) {
    printf("Assign to '%d' value ", RedeByteIterator_nextByte(iterator));
    Rede_printByteCode_type(iterator);
    printf("\n");

    return 0;
}

int Rede_printByteCode_call(RedeByteIterator* iterator) {
    printf("Call function '");
    int nameLength = RedeByteIterator_nextByte(iterator);
    for(int i = 0; i < nameLength; i++) {
        printf("%c", (char)RedeByteIterator_nextByte(iterator));
    }
    printf("' with arguments count %d\n", RedeByteIterator_nextByte(iterator));

    return 0;
}

int Rede_printByteCode_parseJumpSize(RedeByteIterator* iterator) {
    int jumpSize = 0;
    unsigned char* jumpBytes = (unsigned char*)&jumpSize;
    jumpBytes[0] = RedeByteIterator_nextByte(iterator);
    jumpBytes[1] = RedeByteIterator_nextByte(iterator);

    return jumpSize;
}

int Rede_printByteCode_jump(RedeByteIterator* iterator) {
    printf(RedeByteIterator_nextByte(iterator) == 0 ? "Jump forward " : "Jump backward ");

    int jump = Rede_printByteCode_parseJumpSize(iterator);

    printf("%d bytes\n", jump);

    return 0;
}

int Rede_printByteCode_jumpIf(RedeByteIterator* iterator) {
    printf("If ");
    Rede_printByteCode_type(iterator);
    printf(RedeByteIterator_nextByte(iterator) == 0 ? " jump forward " : "jump backward ");

    int jump = Rede_printByteCode_parseJumpSize(iterator);
    printf("%d bytes\n", jump);

    return 0;
}

int Rede_printByteCode_jumpIfNot(RedeByteIterator* iterator) {
    printf("If not ");
    Rede_printByteCode_type(iterator);
    printf(RedeByteIterator_nextByte(iterator) == 0 ? " jump forward " : "jump backward ");
    
    int jump = Rede_printByteCode_parseJumpSize(iterator);
    printf("%d bytes\n", jump);

    return 0;
}

int Rede_printByteCode_stackPush(RedeByteIterator* iterator) {
    printf("Put on the stack ");
    Rede_printByteCode_type(iterator);
    printf("\n");

    return 0;
}

int Rede_printByteCode_stackClear(RedeByteIterator* iterator __attribute__((unused))) {
    printf("Clear the stack\n");
    
    return 0;
}

typedef struct Rede_printByteCode_Printer {
    int (*print)(RedeByteIterator* iterator);
} Rede_printByteCode_Printer;

Rede_printByteCode_Printer Rede_printByteCodePrinters[] = {
    [REDE_CODE_ASSIGN] = { Rede_printByteCode_assignment },
    [REDE_CODE_CALL] = { Rede_printByteCode_call },
    [REDE_CODE_JUMP] = { Rede_printByteCode_jump },
    [REDE_CODE_JUMP_IF] = { Rede_printByteCode_jumpIf },
    [REDE_CODE_JUMP_IF_NOT] = { Rede_printByteCode_jumpIfNot },
    [REDE_CODE_STACK_PUSH] = { Rede_printByteCode_stackPush },
    [REDE_CODE_STACK_CLEAR] = { Rede_printByteCode_stackClear },
};


void Rede_printBytecode(RedeByteCode* code) {
    RedeByteIterator iterator;

    RedeByteIterator_init(code, &iterator);


    unsigned char byte;
    while((byte = RedeByteIterator_nextByte(&iterator)) != REDE_CODE_END) {
        if(byte == REDE_CODE_NOP) {
            printf(" - NOP\n");
        } else if(byte < sizeof(Rede_printByteCodePrinters) / sizeof(Rede_printByteCode_Printer)) {
            Rede_printByteCode_Printer* printer = Rede_printByteCodePrinters + byte;
            printf(" - ");
            if(printer->print(&iterator) < 0) {
                return;
            }
        } else {
            printf("Unknown instruction %d\n", byte);
            return;
        }
    }
    printf(" - END\n");


    RedeByteIterator_destroy(&iterator);
}


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

#endif // REDE_RUNTIME_IMPLEMENTATION
