#if defined(REDE_IMPLEMENTATION)
#define REDE_COMPILER_IMPLEMENTATION
#define REDE_RUNTIME_IMPLEMENTATION
#define REDE_STD_IMPLEMENTATION
#endif // REDE_IMPLEMENTATION
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
    size_t index;
    union {
        struct {
            unsigned char* buffer;
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
        .index = -1,\
        .data = {\
            .buffer = {\
                .buffer = name##__buffer,\
                .maxLength = bufferLength,\
            }\
        }\
    };\
    RedeDest* name = &name##__data;\


#define Rede_createFileDest(name, filePath)\
    RedeDest name##__data = {\
        .type = RedeDestTypeFile,\
        .index = -1,\
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

#if defined(REDE_COMPILER_IMPLEMENTATION)

#include <stdio.h>

#if !defined(REDE_SOURCE_ITERATOR)
#define REDE_SOURCE_ITERATOR

typedef enum RedeSourceIteratorType {
    RedeSourceIteratorTypeString,
    RedeSourceIteratorTypeFile
} RedeSourceIteratorType;

typedef struct RedeSourceIterator {
    size_t index;
    int finished;
    char current;
    RedeSourceIteratorType type;
    union {
        char* string;
        struct {
            FILE* fp;
        } file;
    } data;
} RedeSourceIterator;

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator);
void RedeSourceIterator_destroy(RedeSourceIterator* iterator);

char RedeSourceIterator_nextChar(RedeSourceIterator* iterator);
char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index);
char RedeSourceIterator_current(RedeSourceIterator* iterator);
void RedeSourceIterator_moveCursorBack(RedeSourceIterator* iterator, size_t shift);

#endif // REDE_SOURCE_ITERATOR





#include <stdio.h>
#include <stdlib.h>

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator) {
    iterator->index = -1;
    iterator->finished = 0;
    iterator->current = 0;
    switch(src->type) {
        case RedeSourceTypeString:
            iterator->type = RedeSourceIteratorTypeString;
            iterator->data.string = src->data.string;
            break;

        case RedeSourceTypeFile: {
            FILE* fp = fopen(src->data.path, "r");
            if(!fp) return -1;
            iterator->type = RedeSourceIteratorTypeFile;
            iterator->data.file.fp = fp;
            break;
        }
    }

    return 0;
}

void RedeSourceIterator_destroy(RedeSourceIterator* iterator) {
    if(iterator->type == RedeSourceIteratorTypeFile) {
        fclose(iterator->data.file.fp);
    }
}

char RedeSourceIterator_nextChar(RedeSourceIterator* iterator) {
    if(iterator->finished) return '\0';
    iterator->index++;
    switch(iterator->type) 
        case RedeSourceIteratorTypeString: {
            iterator->current = iterator->data.string[iterator->index];
            if(!iterator->current) iterator->finished = 1;

            break;

        case RedeSourceIteratorTypeFile:
            iterator->current = getc(iterator->data.file.fp);
            if(iterator->current == EOF) {
                iterator->finished = 1;
                iterator->current = '\0';
            }
            
            break;

        default:
            fprintf(stderr, "Unknown iterator type\n");
            exit(1);
    }
    return iterator->current;
}

char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index) {
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: 
            return iterator->data.string[index];

        case RedeSourceIteratorTypeFile: {
            size_t diff = iterator->index - index;
            fseek(iterator->data.file.fp, -diff - 1, SEEK_CUR);
            char ch = getc(iterator->data.file.fp);
            fseek(iterator->data.file.fp, diff, SEEK_CUR);
            
            return ch;
        }

        default:
            fprintf(stderr, "Unknown iterator type\n");
            exit(1);
    }
}

char RedeSourceIterator_current(RedeSourceIterator* iterator) {
    return iterator->current;
}

void RedeSourceIterator_moveCursorBack(RedeSourceIterator* iterator, size_t shift) {
    iterator->index -= shift;
    
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: 
            iterator->current = iterator->data.string[iterator->index];
            break;

        case RedeSourceIteratorTypeFile:
            fseek(iterator->data.file.fp, iterator->index, SEEK_SET);
            iterator->current = getc(iterator->data.file.fp);
            fseek(iterator->data.file.fp, -1, SEEK_CUR);
            break;

        default:
            fprintf(stderr, "Unknown iterator type\n");
            exit(1);
    }
}

#if !defined(LOGS_H)
#define LOGS_H

#include <stdio.h>

#if defined(REDE_DO_LOGS)
    #define LOGS_SCOPE(name)\
        const char* logs__scope__name = #name;\
        if(!logs__scope__name[0]) logs__scope__name = __func__;\
        printf("LOGS '%s'\n", logs__scope__name);\

    #define LOG(...)\
        do {\
            printf("LOGS '%s' ", logs__scope__name);\
            printf(__VA_ARGS__);\
        } while(0);\
    
    #define LOG_LN(...)\
        do {\
            LOG(__VA_ARGS__);\
            printf("\n");\
        } while(0);\

    #define CHECK(condition, ...)\
        do {\
            int LOCAL_STATUS = (condition);\
            if(LOCAL_STATUS < 0) {\
                printf("LOGS '%s' Status: %d  ", logs__scope__name, LOCAL_STATUS);\
                printf(__VA_ARGS__);\
                printf("\n");\
                return LOCAL_STATUS;\
            }\
        } while(0);\

    #define LOGS_ONLY(code) code

    #define CHECK_ELSE(condition, elseCode, ...)\
        do {\
            int CONDITION_VALUE = (condition);\
            if(CONDITION_VALUE < 0) {\
                printf("LOGS '%s' Status: %d  ", logs__scope__name, CONDITION_VALUE);\
                printf(__VA_ARGS__);\
                printf("\n");\
                elseCode;\
            }\
        } while(0);
    
    #define CHECK_RETURN(condition, ...)\
        do {\
            int LOCAL_STATUS = (condition);\
            if(LOCAL_STATUS < 0) {\
                printf("LOGS '%s' Status: %d  ", logs__scope__name, LOCAL_STATUS);\
                printf(__VA_ARGS__);\
                printf("\n");\
            }\
            return LOCAL_STATUS;\
        } while(0);\

#else
    #define LOGS_SCOPE(name)
    #define LOG(...)
    #define LOG_LN(...)

    #define CHECK(condition, ...)\
        do {\
            int LOCAL_STATUS = (condition);\
            if(LOCAL_STATUS < 0) return LOCAL_STATUS;\
        } while(0);\

    #define LOGS_ONLY(code)

    #define CHECK_ELSE(condition, elseCode, ...)\
        do {\
            int CONDITION_VALUE = (condition);\
            if(CONDITION_VALUE < 0) {\
                elseCode;\
            }\
        } while(0);
    
    #define CHECK_RETURN(condition, ...) return (condition);

#endif // REDE_DO_LOGS


#endif // LOGS_H


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




int RedeDest_init(RedeDest* dest) {
    LOGS_SCOPE();

    dest->index = -1;

    switch(dest->type) {
        case RedeDestTypeFile: {
            dest->data.file.fp = fopen(dest->data.file.path, "wb");
            if(!dest->data.file.fp) {
                LOG_LN("Failed to open file '%s'", dest->data.file.path);
                return -1;
            }
            return 0;
        }

        case RedeDestTypeBuffer:
            return 0;

        default:
            LOG_LN("Unknown destination type");
            return -1;
    }
    return 0;
}

void RedeDest_destroy(RedeDest* dest) {
    LOGS_SCOPE();
    if(dest->type == RedeDestTypeFile) {
        fclose(dest->data.file.fp);
    }
}

int RedeDest_writeByte(RedeDest* dest, unsigned char byte) {

    dest->index++;
    switch(dest->type) {
        case RedeDestTypeBuffer:
            if(dest->index != dest->data.buffer.maxLength) {
                dest->data.buffer.buffer[dest->index] = byte;
                return 0;
            } else {
                return -1;
            }

        case RedeDestTypeFile:
            if(fputc(byte, dest->data.file.fp) != byte) {
                return -1;
            };
            return 0;

        default:
            return -1;
    }
}

void RedeDest_moveCursorBack(RedeDest* dest, size_t n) {
    dest->index -= n;
    switch(dest->type) {    
        case RedeDestTypeFile:
            fseek(dest->data.file.fp, -n, SEEK_CUR);
            return;
            
        default:
            return;
    }
}

int RedeDest_writeByteAt(RedeDest* dest, size_t index, unsigned char byte) {
    switch(dest->type) {    
        case RedeDestTypeBuffer:
            if(index >= dest->data.buffer.maxLength) return -1;
            dest->data.buffer.buffer[index] = byte;
            return 0;

        case RedeDestTypeFile: 
            fseek(dest->data.file.fp, index, SEEK_SET);
            if(fputc(byte, dest->data.file.fp) != byte) {
                return -1;
            }
            fseek(dest->data.file.fp, dest->index + 1, SEEK_SET);
            return 0;
            
        default:
            return -1;
    }
}




#if !defined(REDE_COMPILER_HELPERS)
#define REDE_COMPILER_HELPERS

/* EOI = End of input */

typedef enum RedeWriteStatus {
    RedeWriteStatusError = -1,
    RedeWriteStatusOk = 0,

    /**
     * @brief Statement ended with bracket
     * 
     */
    RedeWriteStatusBracketTerminated = 1,

    /**
     * @brief Statement ended with EOI
     * 
     */
    RedeWriteStatusEOI = 2
} RedeWriteStatus;

typedef enum RedeExpressionWriteStatus {
    RedeExpressionWriteStatusOk = 0,
    RedeExpressionWriteStatusError = -1,

    /**
     * @brief Expression is a function call
     * 
     */
    RedeExpressionWriteStatusFunction = 1,

    /**
     * @brief Expression ended with bracket
     * 
     */
    RedeExpressionWriteStatusBracketTerminated = 2,

    /**
     * @brief Expression ended with EOI
     * 
     */
    RedeExpressionWriteStatusEOI = 3,

    /**
     * @brief Got just closing bracket as brackets block end
     * 
     */
    RedeExpressionWriteStatusBracket = 4,
} RedeExpressionWriteStatus;

typedef struct RedeCompilationContextWhileLoop {
    size_t loopStart;
    size_t breakJumpStart;
    int breakRequired;
} RedeCompilationContextWhileLoop;

typedef struct RedeCompilationContext {
    int functionCallDepth;
    int isAssignment;
    int isIfStatementArgument;
    int isWhileLoopArgument;
    int bracketsBlockDepth;
    RedeCompilationContextWhileLoop* whileLoopCtx;
} RedeCompilationContext;

RedeExpressionWriteStatus RedeCompilerHelpers_writeFloat(
    char firstChar, 
    RedeSourceIterator* iterator, RedeDest* dest, RedeCompilationContext* ctx
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeString(
    int singleQuoted, 
    RedeSourceIterator* iterator, RedeDest* dest
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeVariableValue(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeOperationWithToken(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeBoolean(int value, RedeDest* dest);

RedeExpressionWriteStatus RedeCompilerHelpers_writeExpression(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeAssignment(
    size_t tokenStart, size_t tokenLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeFunctionCall(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
);

RedeWriteStatus RedeCompilerHelpers_writeStatements(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeStatement(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeWhile(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeContinue(RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeBreak(RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeIfStatement(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeElseStatement(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);


RedeWriteStatus RedeCompilerHelpers_parseComment(RedeSourceIterator* src);


unsigned long RedeCompilerHelpers_hash(RedeSourceIterator* iterator, size_t identifierStart, size_t identifierLength);

int RedeCompilerHelpers_isToken(char* token, size_t identifierStart, size_t identifierLength, RedeSourceIterator* iterator);
int RedeCompilerHelpers_nextTokenIs(char* token, RedeSourceIterator* iterator);
#endif // REDE_COMPILER_HELPERS



RedeWriteStatus RedeCompilerHelpers_parseComment(RedeSourceIterator* src) {
    LOGS_SCOPE(parseComment);

    char ch;
    while((ch = RedeSourceIterator_nextChar(src))) {
        LOG_LN("Char: '%c'(%d)", ch, ch);

        int end = 0;

        switch(ch) {
            case '\r':
            case '\n':
            case '#':
                LOG_LN("End of the comment");
                end = 1;
                break;
        }

        if(end) break;
    }

    return RedeWriteStatusOk;
}


unsigned long RedeCompilerHelpers_hash(
    RedeSourceIterator* iterator, 
    size_t identifierStart, 
    size_t identifierLength
) {
    LOGS_SCOPE(hash);

    unsigned long hash = 5381;
    for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("%zu) '%c'(%d)", i, ch, ch);
        hash = ((hash << 5) + hash) + ch; /* hash * 33 + c */
    }

    return hash;
}

int RedeCompilerHelpers_isToken(char* token, size_t tokenStart, size_t tokenLength, RedeSourceIterator* iterator) {

    size_t i = 0;
    while(1) {
        if(i == tokenLength) {
            return token[i] == '\0';
        }

        char tokenChar = token[i];
        char strChar = RedeSourceIterator_charAt(iterator, tokenStart + i);
        
        if(tokenChar == '\0' || tokenChar != strChar) return 0;
        
        i++;
    }
}

int RedeCompilerHelpers_nextTokenIs(char* token, RedeSourceIterator* iterator) {
    LOGS_SCOPE(nextTokenIs);

    LOG_LN("Checking for token '%s'", token);

    size_t tokenIndex = 0;
    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("Char: %c(%d)", ch, ch);

        if(ch == ' ' || ch == '\n' || ch == '\r' || ch == 9/* TAB */) {
            if(tokenIndex > 0) {
                LOG_LN("Mismatch, reverting %zu token chars", tokenIndex + 1)
                RedeSourceIterator_moveCursorBack(iterator, tokenIndex + 1);
                return 0;
            }

            continue;
        } else if(ch == token[tokenIndex]) {
            tokenIndex++;
            if(token[tokenIndex] == '\0') return 1;
        } else {
            LOG_LN("Mismatch, reverting %zu token chars", tokenIndex + 1)
            RedeSourceIterator_moveCursorBack(iterator, tokenIndex + 1);
            
            return 0;
        }
    }

    return 0;
}

RedeExpressionWriteStatus RedeCompilerHelpers_writeBoolean(int value, RedeDest* dest) {
    LOGS_SCOPE(writeBoolean);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_BOOL), "Failed to write REDE_TYPE_BOOL");
    CHECK(RedeDest_writeByte(dest, value == 0 ? 0 : 1), "Failed to write boolean value");

    return RedeExpressionWriteStatusOk;
}


RedeWriteStatus RedeCompilerHelpers_writeAssignment(
    size_t tokenStart, size_t tokenLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeAssignment);
    ctx->isAssignment = 1;

    CHECK(RedeDest_writeByte(dest, REDE_CODE_ASSIGN), "Failed to write REDE_CODE_ASSIGN to the buffer");

    unsigned long arrayIndex = RedeCompilerHelpers_hash(iterator, tokenStart, tokenLength) % memory->variables.bufferSize;
    LOG_LN("VARIABLE_HASH_TABLE_INDEX: %zu", arrayIndex);

    RedeVariableName* name = memory->variables.buffer + arrayIndex;

    if(!name->isBusy) {
        name->isBusy = 1;
        name->index = memory->variables.nextIndex++;
        name->start = tokenStart;
        name->length = tokenLength;
        LOG_LN("Registering new variable with index %d", name->index);
    }
    LOGS_ONLY(
        else {
            LOG_LN("Variable already exist, index = %d", name->index);
        }
    )

    CHECK(RedeDest_writeByte(dest, name->index), "Failed to write variable index '%d' to the buffer", name->index);

    int status = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);

    CHECK(status, "Failed to write expression");

    if(status == RedeExpressionWriteStatusFunction) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_ASSIGN), "Failed to write REDE_CODE_ASSIGN to the buffer after function call");
        CHECK(RedeDest_writeByte(dest, name->index), "Failed to write variable index '%d' to the buffer after function call", name->index);
        CHECK(RedeDest_writeByte(dest, REDE_TYPE_STACK), "Failed to write REDE_TYPE_STACK after function call");
    }

    ctx->isAssignment = 0;
    
    switch(status) {
        case RedeExpressionWriteStatusBracketTerminated:
            return RedeWriteStatusBracketTerminated;
        
        case RedeExpressionWriteStatusEOI:
            return RedeWriteStatusEOI;
        
        default:
            return RedeWriteStatusOk;
    }
}


RedeWriteStatus RedeCompilerHelpers_writeBreak(
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeBreak);

    if(!ctx->whileLoopCtx) {
        LOG_LN("break keyword used outside of while-loop");
        return RedeWriteStatusError;
    }
    ctx->whileLoopCtx->breakRequired = 1;

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP), "Failed to write REDE_CODE_JUMP");
    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_BACKWARD), "Failed to write REDE_DIRECTION_BACKWARD");
    
    size_t bytesDiff = dest->index - ctx->whileLoopCtx->breakJumpStart + 1;
    if(bytesDiff > 0xFFFF) {
        LOG_LN("The loop is to big to jump backward");
        return RedeWriteStatusError;
    }

    LOG_LN("Back jump length: %zu", bytesDiff);

    unsigned char* bytes = (unsigned char*)&bytesDiff;

    CHECK(RedeDest_writeByte(dest, bytes[0]), "Failed to write the first byte of the back jump");
    CHECK(RedeDest_writeByte(dest, bytes[1]), "Failed to write the second byte of the back jump");


    return RedeWriteStatusOk;
}


RedeWriteStatus RedeCompilerHelpers_writeContinue(
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeContinue);

    if(!ctx->whileLoopCtx) {
        LOG_LN("continue keyword used outside of while-loop");
        return RedeWriteStatusError;
    }

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP), "Failed to write REDE_CODE_JUMP");
    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_BACKWARD), "Failed to write REDE_DIRECTION_BACKWARD");
    
    size_t bytesDiff = dest->index - ctx->whileLoopCtx->loopStart + 1;
    if(bytesDiff > 0xFFFF) {
        LOG_LN("The loop is to big to jump backward");
        return RedeWriteStatusError;
    }

    LOG_LN("Back jump length: %zu", bytesDiff);

    unsigned char* bytes = (unsigned char*)&bytesDiff;

    CHECK(RedeDest_writeByte(dest, bytes[0]), "Failed to write the first byte of the back jump");
    CHECK(RedeDest_writeByte(dest, bytes[1]), "Failed to write the second byte of the back jump");

    return RedeWriteStatusOk;
}


RedeWriteStatus RedeCompilerHelpers_writeElseStatement(
    RedeSourceIterator* src,
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeElseStatement);
    
    if(!RedeCompilerHelpers_nextTokenIs("else", src)) {
        LOG_LN("Not 'else' token");
        return RedeWriteStatusOk;
    }

    LOG_LN("Else statement");

    // Jump after if-statement to skip the else body
    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP), "Failed to write REDE_CODE_JUMP after if-statement");
    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_FORWARD), "Failed to write REDE_DIRECTION_FORWARD after if-statement");
    CHECK(RedeDest_writeByte(dest, 0), "Failed to write jump size first byte placeholder after if-statement");
    size_t jumpSizeStart = dest->index;
    CHECK(RedeDest_writeByte(dest, 0), "Failed to write jump size second byte placeholder after if-statement");

    int isMultipleStatements = RedeCompilerHelpers_nextTokenIs("(", src);

    RedeWriteStatus resultStatus = RedeWriteStatusOk;

    if(isMultipleStatements) {
        LOG_LN("Multiple statements");

        ctx->bracketsBlockDepth++;
        CHECK(RedeCompilerHelpers_writeStatements(src, memory, dest, ctx), "failed to parse statements");
        ctx->bracketsBlockDepth--;
    } else {
        LOG_LN("Single statement");
        CHECK(resultStatus = RedeCompilerHelpers_writeStatement(src, memory, dest, ctx), "Failed to parse statement");
    }

    size_t diff = dest->index - (jumpSizeStart + 1);

    LOG_LN("Jump size: %zu bytes", diff);

    if(diff > 0xFFFF) {
        LOG_LN("Else body is too big to jump over");

        return RedeWriteStatusError;
    }

    unsigned char* diffBytes = (unsigned char*)&diff;

    CHECK(RedeDest_writeByteAt(dest, jumpSizeStart + 0, diffBytes[0]), "Failed to write jump size first byte");
    CHECK(RedeDest_writeByteAt(dest, jumpSizeStart + 1, diffBytes[1]), "Failed to write jump size second byte");

    return resultStatus;
}


RedeExpressionWriteStatus RedeCompilerHelpers_writeExpression(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeExpression);

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);
        
        if((ch >= '0' && ch <= '9') || ch == '-') {
            LOG_LN("Number assignment");
            CHECK_RETURN(RedeCompilerHelpers_writeFloat(ch, iterator, dest, ctx), "Failed to write a float");
        } else if(ch == '"' || ch == '\'') {
            LOG_LN("String assignment");
            CHECK_RETURN(RedeCompilerHelpers_writeString(ch == '\'', iterator, dest), "Failed to write a string");
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            LOG_LN("Operation with token");
            CHECK_RETURN(RedeCompilerHelpers_writeOperationWithToken(iterator, memory, dest, ctx), "Failed to write function call or variable value");
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        } else if(ctx->bracketsBlockDepth > 0 && ch == ')') {
            LOG_LN("Brackets block end");
            
            return RedeExpressionWriteStatusBracket;
        } else if (ch == '#') {
            LOG_LN("Comment start");
            RedeCompilerHelpers_parseComment(iterator);
        } else {
            LOG_LN("Unexpected token");
            return RedeExpressionWriteStatusError;
        }
    }

    LOG_LN("Unexpected end of input");
    return RedeExpressionWriteStatusError;
}


static size_t RedeCompilerHelpers_pow10L(size_t power) {
    size_t result = 1;

    for(size_t i = 0; i < power; i++) {
        result *= 10;
    }

    return result;
}

RedeExpressionWriteStatus RedeCompilerHelpers_writeFloat(
    char firstChar, 
    RedeSourceIterator* iterator,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeFloat);

    int isNegative = firstChar == '-';
    float result;
    if(isNegative) {
        LOG_LN("Negative number");
        result = 0;
    } else {
        LOG_LN("Positive number");
        result = firstChar - '0';
    }

    int floatingPoint = 0;
    size_t floatingPointPosition = 1;

    int endedWithSeparator = 0;
    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);
        if(ch >= '0' && ch <= '9') {
            if(!floatingPoint) {
                result *= 10;
                result += ch - '0';
            } else {
                result += (float)(ch - '0') / RedeCompilerHelpers_pow10L(floatingPointPosition);
                floatingPointPosition++;
            }
        } else if(ch == '.' && !floatingPoint) {
            LOG_LN("Floating point");
            floatingPoint = 1;
        } else if(
            ch == ' ' || ch == '\n' || ch == '\r' 
            ||
            (ctx->bracketsBlockDepth > 0 && ch == ')')
        ) {
            LOGS_ONLY(
                if(ctx->bracketsBlockDepth > 0 && ch == ')') {
                    LOG_LN("Count as input end because of brackets block depth = %d", ctx->bracketsBlockDepth);
                }
            )
            endedWithSeparator = 1;
            break;
        } else {
            LOG_LN("Unexpected character");
            return RedeExpressionWriteStatusError;
        }
    }

    if(isNegative) {
        result *= -1;
    }

    LOG_LN("Result: %f", result);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_NUMBER), "Failed to write REDE_TYPE_NUMBER");


    LOG_LN("Serializing");

    char* bytes = (char*)&result;

    for(size_t i = 0; i < sizeof(float); i++) {
        CHECK(RedeDest_writeByte(dest, bytes[i]), "Failed to write float byte with index %zu", i);
    }
    
    if(ch == ')') {
        return RedeExpressionWriteStatusBracketTerminated;
    } else if(!endedWithSeparator) {
        return RedeExpressionWriteStatusEOI;
    } else {
        return RedeExpressionWriteStatusOk;
    }
}


RedeExpressionWriteStatus RedeCompilerHelpers_writeFunctionCall(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeFunctionCall);
    ctx->functionCallDepth++;

    LOG_LN("Current function call depth: %d", ctx->functionCallDepth);

    if(ctx->isAssignment && ctx->functionCallDepth == 1) {
        LOG_LN("Shifting the buffer cursor back because of function call inside of assignment");
        RedeDest_moveCursorBack(dest, 2);
    } else if(ctx->isWhileLoopArgument && ctx->functionCallDepth == 1) {
        LOG_LN("Shifting the buffer cursor back because of function call as while-loop argument");
        RedeDest_moveCursorBack(dest, 1);
    } else if(ctx->isIfStatementArgument && ctx->functionCallDepth == 1) {
        LOG_LN("Shifting the buffer cursor back because of function call as if-statement argument");
        RedeDest_moveCursorBack(dest, 1);
    } else if(ctx->functionCallDepth > 1) {
        LOG_LN("Shifting the buffer cursor back because of function call inside of function call");
        RedeDest_moveCursorBack(dest, 1);
    }


    LOG("Identifier (s: %zu, l: %zu)", identifierStart, identifierLength);
    LOGS_ONLY(
        for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
            printf(" '%c'", RedeSourceIterator_charAt(iterator, i));
        }
        printf("\n");
    )
    
    ctx->bracketsBlockDepth++;
    size_t argc = 0;
    while(1) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_STACK_PUSH), "Failed to write REDE_CODE_STACK_PUSH");
        int status = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);
        CHECK(status, "Failed to write parameter with index %zu", argc - 1);

        if(status == RedeExpressionWriteStatusBracketTerminated) {
            LOG_LN("Got expression closing bracket status: End of arguments");
            argc++;
            break;
        } else if(status == RedeExpressionWriteStatusBracket) {
            RedeDest_moveCursorBack(dest, 1);
            break;
        } else {
            argc++;
        }
    }
    ctx->bracketsBlockDepth--;

    LOG_LN("Arguments length: %zu", argc);

    if(identifierLength > 255) {
        LOG_LN("Identifier length is too big: %zu > 255", identifierLength);
        return RedeExpressionWriteStatusError;
    }

    CHECK(RedeDest_writeByte(dest, REDE_CODE_CALL), "Failed to write REDE_CODE_CALL");
    CHECK(RedeDest_writeByte(dest, (unsigned char)identifierLength), "Failed to write identifier length");

    LOG_LN("Writing identifier: ");
    for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        CHECK(RedeDest_writeByte(dest, ch), "Failed to write");
    }

    if(argc > 255) {
        LOG_LN("Too much parameters: %zu > 255", argc);
        return RedeExpressionWriteStatusError;
    }

    CHECK(RedeDest_writeByte(dest, (unsigned char)argc), "Failed to write arguments count");

    ctx->functionCallDepth--;

    return RedeExpressionWriteStatusFunction;
}


RedeExpressionWriteStatus RedeCompilerHelpers_writeOperationWithToken(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeOperationWithToken);

    size_t identifierStart = iterator->index;
    size_t identifierLength = 1;

    char ch;
    while(1) {
        ch = RedeSourceIterator_nextChar(iterator);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        int isBracketSeparator = ctx->bracketsBlockDepth > 0 && ch == ')';

        if(
            ch == ' ' || ch == '\n' || ch == '\r'  || ch == '\0'
            || 
            isBracketSeparator
        ) {
            LOGS_ONLY(
                if(isBracketSeparator) {
                    LOG_LN("Token separated by ')' token at the end of brackets block");
                } else if(ch == '\0') {
                    LOG_LN("Input end");
                }
            );

            if(RedeCompilerHelpers_isToken("true", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'true'");
                CHECK(RedeCompilerHelpers_writeBoolean(1, dest), "Failed to write boolean");
            } else if(RedeCompilerHelpers_isToken("false", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'false'");
                CHECK(RedeCompilerHelpers_writeBoolean(0, dest), "Failed to write boolean");
            } else {
                LOG_LN("Variable value");
                CHECK(RedeCompilerHelpers_writeVariableValue(identifierStart, identifierLength, iterator, memory, dest), "Failed to write variable value");
            }

            if(ch == '\0') {
                return RedeExpressionWriteStatusEOI;
            } else if(isBracketSeparator) {
                return RedeExpressionWriteStatusBracketTerminated;
            } else {
                return RedeExpressionWriteStatusOk;
            }
        } else if(ch == '(') {
            LOG_LN("Function call");
            CHECK_RETURN(RedeCompilerHelpers_writeFunctionCall(identifierStart, identifierLength, iterator, memory, dest, ctx), "Failed to write function call");
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            identifierLength++;
        } else {
            LOG_LN("Unexpected char");

            return RedeExpressionWriteStatusError;
        }
    }

    LOG_LN("Unexpected loop break");
    return RedeExpressionWriteStatusError;
}


RedeWriteStatus RedeCompilerHelpers_writeStatement(
    RedeSourceIterator* iterator,
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeStatement);

    int lookingForTokenStart = 1;
    size_t tokenStart = 0;
    size_t tokenLength = 0;
    int tokenEnded = 0;

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("Char: '%c'(%d)", ch, ch);

        int isBracketSeparator = ctx->bracketsBlockDepth > 0 && ch == ')';


        if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) { // Registering token start
            if(tokenEnded) {
                LOG_LN("Unexpected char '%c', expected function call or assignment", ch);
                return -1;
            }
            if(lookingForTokenStart) {
                lookingForTokenStart = 0;
                tokenStart = iterator->index;
                tokenLength = 0;
                tokenEnded = 0;
            }
            tokenLength++;
        } else if( // Registering token end and handling keywords or just skipping the white spaces
            ch == ' ' || ch == '\n' || ch == '\r' 
            ||
            isBracketSeparator
        ) {
            RedeWriteStatus status = -20; // Default value
            if(!tokenEnded && !lookingForTokenStart) { // If we are actually looking for token end

                tokenEnded = 1;

                if(RedeCompilerHelpers_isToken("continue", tokenStart, tokenLength, iterator)) {
                    LOG_LN("Keyword: continue");

                    CHECK(status = RedeCompilerHelpers_writeContinue(dest, ctx), "Failed to write continue");
                } else if(RedeCompilerHelpers_isToken("break", tokenStart, tokenLength, iterator)) {
                    LOG_LN("Keyword: break");

                    CHECK(status = RedeCompilerHelpers_writeBreak(dest, ctx), "Failed to write break");
                } else if(RedeCompilerHelpers_isToken("while", tokenStart, tokenLength, iterator)) {
                    LOG_LN("While loop");

                    CHECK(status = RedeCompilerHelpers_writeWhile(iterator, memory, dest, ctx), "Failed to write while");
                } else if(RedeCompilerHelpers_isToken("if", tokenStart, tokenLength, iterator)) {
                    LOG_LN("If statement");

                    CHECK(status = RedeCompilerHelpers_writeIfStatement(iterator, memory, dest, ctx), "Failed to write if-statement");
                } else if(RedeCompilerHelpers_isToken("else", tokenStart, tokenLength, iterator)) {
                    LOG_LN("Unexpected 'else' keyword");

                    return RedeWriteStatusError;
                }
            }

            if(isBracketSeparator) {
                LOG_LN("Got ')' inside of brackets block. End of the block");
                return RedeWriteStatusBracketTerminated;
            }
            
            if(status != -20) {
                return status;
            }
        } else if(ch == '=' || ch == '(') { // Handling assignments and function calls
            if(tokenLength == 0) {
                LOG_LN("Unexpected char");

                return RedeWriteStatusError;
            }
            // TODO: add keywords handling

            LOGS_ONLY(
                LOG("Token:");
                for(size_t i = tokenStart; i < tokenStart + tokenLength; i++) {
                    printf(" '%c'", RedeSourceIterator_charAt(iterator, i));
                }
                printf("\n");
            );
            if(ch == '=') {
                LOG_LN("Variable assignment");
                CHECK_RETURN(RedeCompilerHelpers_writeAssignment(tokenStart, tokenLength, iterator, memory, dest, ctx), "Failed to write assignment");
            } else {
                LOG_LN("Function call");
                CHECK(RedeCompilerHelpers_writeFunctionCall(tokenStart, tokenLength, iterator, memory, dest, ctx), "Failed to write function call");
                CHECK(RedeDest_writeByte(dest, REDE_CODE_STACK_CLEAR), "Failed to clear the stack");
                return RedeWriteStatusOk;
            }
        } else if(ch == '#') { // parsing comments
            LOG_LN("Comment start")
            RedeCompilerHelpers_parseComment(iterator);
        } else {
            LOG_LN("Unexpected char");
            return RedeWriteStatusError;
        }
    }

    if(lookingForTokenStart && ctx->bracketsBlockDepth == 0) {
        LOG_LN("Got input end");
        return RedeWriteStatusEOI;
    }
     
    LOG_LN("Unexpected end of input");
    return RedeWriteStatusError;
}

// Iterate over the source and write statements one by one
RedeWriteStatus RedeCompilerHelpers_writeStatements(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeStatementS);
    
    while(1) {
        int status = RedeCompilerHelpers_writeStatement(iterator, memory, dest, ctx);
        CHECK(status, "Failed to write a statement");

        // We do essentials the same but with different statement terminators
        if(status == RedeWriteStatusBracketTerminated) {
            LOG_LN("Last statement was bracket terminated");
            return RedeWriteStatusOk;
        } else if(status == RedeWriteStatusEOI) {
            LOG_LN("Last statement ended with EOI");
            return RedeWriteStatusOk;
        }
    }

    return RedeWriteStatusOk;
}


RedeExpressionWriteStatus RedeCompilerHelpers_writeString(
    int singleQuoted, 
    RedeSourceIterator* iterator,
    RedeDest* dest
) {
    LOGS_SCOPE(writeString);

    LOG_LN(singleQuoted ? "Single quoted" : "Double quoted");


    size_t stringStart = iterator->index + 1;
    size_t pureStringLength = 0;
    size_t iteratedChars = 0;

    int endedWithQuotes = 0;
    int isBackSlashed = 0;

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(isBackSlashed) {
            LOG_LN("Back-slashed");
            isBackSlashed = 0;
            pureStringLength++;
        } else if((singleQuoted && ch == '\'') || (!singleQuoted && ch == '"')) {
            LOG_LN("String end");

            endedWithQuotes = 1;

            break;
        } else if (ch == '\\') {
            LOG_LN("Back-slash mark");
            
            isBackSlashed = 1;
        } else {
            pureStringLength++;
        }
        
        iteratedChars++;
    }

    if(!endedWithQuotes) {
        LOG_LN("Unexpected end of the string");

        return RedeExpressionWriteStatusError;
    }

    LOG_LN("Chars iterated: %zu", iteratedChars);
    LOG_LN("String length: %zu", pureStringLength);


    if(pureStringLength > 255) {
        LOG_LN("String length overflow (%zu > 255)", pureStringLength);

        return RedeExpressionWriteStatusError;
    }

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_STRING), "Failed to write REDE_TYPE_STRING");

    CHECK(RedeDest_writeByte(dest, (unsigned char)pureStringLength), "Failed to write string length");

    LOG_LN("Writing to the buffer");
    for(size_t i = stringStart; i < stringStart + iteratedChars; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(ch != '\\') {
            CHECK(RedeDest_writeByte(dest, (unsigned char)ch), "Failed to write char");
        } 
        LOGS_ONLY(
            else {
                LOG_LN("Skiped as back-slash")
            }
        )
    }

    return 0;
}


RedeExpressionWriteStatus RedeCompilerHelpers_writeVariableValue(
    size_t identifierStart, 
    size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest
) {
    LOGS_SCOPE(writeVariableValue);

    LOG("Identifier (s: %zu, l: %zu)", identifierStart, identifierLength);
    LOGS_ONLY(
        for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
            printf(" '%c'", RedeSourceIterator_charAt(iterator, i));
        }
        printf("\n");
    )

    unsigned long hashTableIndex = RedeCompilerHelpers_hash(iterator, identifierStart, identifierLength) % memory->variables.bufferSize;

    LOG_LN("Hash table array index: %zu", hashTableIndex);

    RedeVariableName* name = memory->variables.buffer + hashTableIndex;

    if(!name->isBusy) {
        LOG_LN("Variable is not defined");

        return RedeExpressionWriteStatusError;
    }

    LOG_LN("Variable index: %d", name->index);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_VAR), "Failed to write REDE_TYPE_VAR");

    CHECK(RedeDest_writeByte(dest, name->index), "Failed to write variable index");

    return RedeExpressionWriteStatusOk;
}


RedeWriteStatus RedeCompilerHelpers_writeWhile(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeWhile);
    RedeCompilationContextWhileLoop currentLoop = {
        .breakRequired = 0
    };

    size_t preBreakJumpStart;

    for(int i = 0; i < 8; i++) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_NOP), "Failed write placeholder nop at index %d", i);
        if(i == 0) {
            preBreakJumpStart = dest->index;
        } else if(i == 4) {
            currentLoop.breakJumpStart = dest->index;
        }
    }

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), "Failed to write REDE_CODE_JUMP_IF_NOT");

    currentLoop.loopStart = dest->index;


    ctx->isWhileLoopArgument = 1;
        int expressionStatus = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);
        CHECK(expressionStatus, "Failed to write condition");
    ctx->isWhileLoopArgument = 0;

    if(expressionStatus == RedeExpressionWriteStatusFunction) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), "Failed to write REDE_CODE_JUMP_IF_NOT after the function call");
        CHECK(RedeDest_writeByte(dest, REDE_TYPE_STACK), "Failed to write REDE_TYPE_STACK after the function call");
    }

    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_FORWARD), "Failed to write REDE_DIRECTION_FORWARD");

    CHECK(RedeDest_writeByte(dest, 0), "Failed to write the first byte of jump size");
    size_t jumpSizeStart = dest->index;

    CHECK(RedeDest_writeByte(dest, 0), "Failed to write the second byte of jump size");


    RedeCompilationContextWhileLoop* prevCtx = ctx->whileLoopCtx;

    ctx->whileLoopCtx = &currentLoop;


    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("Char: '%c'(%d)", ch, ch);

        int isOpenBracket = ch == '(';
        if(
            (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
            ||
            isOpenBracket
        ) {
            RedeWriteStatus resultStatus = RedeWriteStatusOk;

            if(isOpenBracket) {
                LOG_LN("Multiple statements");

                ctx->bracketsBlockDepth++;
                    CHECK(RedeCompilerHelpers_writeStatements(iterator, memory, dest, ctx), "Failed to write while-loop multiple statements");
                ctx->bracketsBlockDepth--;
            } else {
                LOG_LN("Single statement");

                RedeSourceIterator_moveCursorBack(iterator, 1);
                resultStatus = RedeCompilerHelpers_writeStatement(iterator, memory, dest, ctx);
                CHECK(resultStatus, "Failed to write while-loop single statement");
            }
            
            CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP), "Failed to write REDE_CODE_JUMP");
            CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_BACKWARD), "Failed to write REDE_DIRECTION_BACKWARD");

            size_t bytesDiff = dest->index - currentLoop.loopStart + 1;
            if(bytesDiff > 0xFFFF) {
                LOG_LN("The loop is to big to jump backward");
                return RedeWriteStatusError;
            }

            LOG_LN("Back jump length: %zu", bytesDiff);

            unsigned char* bytes = (unsigned char*)&bytesDiff;

            CHECK(RedeDest_writeByte(dest, bytes[0]), "Failed to write the first byte of the back jump");
            CHECK(RedeDest_writeByte(dest, bytes[1]), "Failed to write the second byte of the back jump");

            bytesDiff = dest->index - jumpSizeStart - 1;

            LOG_LN("Forward jump length: %zu", bytesDiff);

            if(bytesDiff > 0xFFFF) {
                LOG_LN("The loop is to big to jump forward");
                return RedeWriteStatusError;
            }

            CHECK(RedeDest_writeByteAt(dest, jumpSizeStart, bytes[0]), "Failed to write the first byte of the forward jump");
            CHECK(RedeDest_writeByteAt(dest, jumpSizeStart + 1, bytes[1]), "Failed to write the second byte of the forward jump");

            ctx->whileLoopCtx = prevCtx;

            if(currentLoop.breakRequired) {
                LOG_LN("Break required");

                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 0, REDE_CODE_JUMP), "Failed to write REDE_CODE_JUMP for break");
                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 1, REDE_DIRECTION_FORWARD), "Failed to write REDE_DIRECTION_FORWARD for break");
                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 2, 4), "Failed to write first destination byte for break");
                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 3, 0), "Failed to write second destination byte for break");

                bytesDiff = dest->index - preBreakJumpStart - 7;

                LOG_LN("Break jump length: %zu", bytesDiff);

                if(bytesDiff > 0xFFFF) {
                    LOG_LN("The loop is to big to jump forward");
                    return RedeWriteStatusError;
                }

                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 4, REDE_CODE_JUMP), "Failed to write REDE_CODE_JUMP for break itself");
                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 5, REDE_DIRECTION_FORWARD), "Failed to write REDE_DIRECTION_FORWARD for break itself");
                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 6, bytes[0]), "Failed to write first destination byte for break itself");
                CHECK(RedeDest_writeByteAt(dest, preBreakJumpStart + 7, bytes[1]), "Failed to write second destination byte for break itself");
            }

            return resultStatus;
        } else if(ch != ' ' && ch != '\n' && ch != '\r') {
            LOG_LN("Unexpected character");
            return RedeWriteStatusError;
        }
    }

    LOG_LN("Unexpected end of the input");

    return RedeWriteStatusError;
}


RedeWriteStatus RedeCompilerHelpers_writeIfStatement(
    RedeSourceIterator* iterator,
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeIfStatement);

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), "Failed to write REDE_CODE_JUMP_IF_NOT");

    ctx->isIfStatementArgument = 1;
        int status = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);
        CHECK(status, "Failed to write the condition")
    ctx->isIfStatementArgument = 0;

    if(status == RedeExpressionWriteStatusFunction) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), "Failed to write REDE_CODE_JUMP_IF_NOT after the function call");
        CHECK(RedeDest_writeByte(dest, REDE_TYPE_STACK), "Failed to write REDE_TYPE_STACK after the function call");
    }

    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_FORWARD), "Failed to write REDE_DIRECTION_FORWARD");
    CHECK(RedeDest_writeByte(dest, 0), "Failed to write jump length placeholder first byte");
    size_t firstJumpSizeByte = dest->index;
    CHECK(RedeDest_writeByte(dest, 0), "Failed to write jump length placeholder second byte");



    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("Char: '%c'(%d)", ch, ch);


        int isOpenBracket = ch == '(';
        if(
            (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
            ||
            ch == '('
        ) {
            RedeWriteStatus resultStatus = RedeWriteStatusOk;

            if(isOpenBracket) {
                LOG_LN("Multiple statements");

                ctx->bracketsBlockDepth++;
                CHECK(RedeCompilerHelpers_writeStatements(iterator, memory, dest, ctx), "Failed to write if multiple statements");
                ctx->bracketsBlockDepth--;
            } else {
                LOG_LN("Single statement");
                
                RedeSourceIterator_moveCursorBack(iterator, 1);
                resultStatus = RedeCompilerHelpers_writeStatement(iterator, memory, dest, ctx);
                CHECK(resultStatus, "Failed to write if single statement");
            }

            size_t diff = dest->index - (firstJumpSizeByte + 1);
            if(diff > 0xFFFF) {
                LOG_LN("The if body is to big to jump over");
                return RedeWriteStatusError;
            }

            LOG_LN("Jump size: %zu", diff);

            unsigned char* diffBytes = (unsigned char*)&diff;

            CHECK(RedeDest_writeByteAt(dest, firstJumpSizeByte + 0, diffBytes[0]), "Failed to write jump size first byte");
            CHECK(RedeDest_writeByteAt(dest, firstJumpSizeByte + 1, diffBytes[1]), "Failed to write jump size second byte");


            RedeWriteStatus continuationStatus;
            size_t lastIndex = dest->index;
            CHECK(continuationStatus = RedeCompilerHelpers_writeElseStatement(iterator, memory, dest, ctx), "Failed to write else statement");

            if(lastIndex != dest->index) {
                LOG_LN("Got else statement so need to adjust REDE_CODE_JUMP_IF_NOT jump size"); // to handle new 4 jump bytes

                diff += 4;

                LOG_LN("New jump size: %zu", diff);

                if(diff > 0xFFFF) {
                    LOG_LN("The if body is to big to jump over after adjustment");
                    return RedeWriteStatusError;
                }

                CHECK(RedeDest_writeByteAt(dest, firstJumpSizeByte + 0, diffBytes[0]), "Failed to adjust jump size first byte");
                CHECK(RedeDest_writeByteAt(dest, firstJumpSizeByte + 1, diffBytes[1]), "Failed to adjust jump size second byte");

                return continuationStatus;
            }

            return resultStatus;
        }
    }


    LOG_LN("Unexpected end of the input");
    return RedeWriteStatusError;
}



#include <stdio.h>
#include <stdlib.h>


#define EXIT_COMPILER(code)\
    compilationStatus = code;\
    goto exit_compiler;\

int Rede_compile(RedeSource* src, RedeCompilationMemory* memory, RedeDest* dest) {
    LOGS_SCOPE();
    int compilationStatus = 0;

    RedeCompilationContext ctx = {
        .isAssignment = 0,
        .functionCallDepth = 0,
        .isIfStatementArgument = 0,
        .isWhileLoopArgument = 0,
        .whileLoopCtx = NULL,
        .bracketsBlockDepth = 0,
    };

    RedeSourceIterator iterator;
    if(RedeSourceIterator_init(src, &iterator) < 0) {
        LOG_LN("Failed to create iterator");
        return -1;
    };

    if(RedeDest_init(dest) < 0) {
        LOG_LN("Failed to init destination");
        EXIT_COMPILER(-1);
    }


    CHECK_ELSE(
        RedeCompilerHelpers_writeStatements(&iterator, memory, dest, &ctx),
        EXIT_COMPILER(CONDITION_VALUE),
        "Failed to parse statements"
    );


    CHECK_ELSE(
        RedeDest_writeByte(dest, REDE_CODE_END), 
        EXIT_COMPILER(CONDITION_VALUE), 
        "Failed to write REDE_CODE_END"
    );


exit_compiler:
    RedeSourceIterator_destroy(&iterator);
    RedeDest_destroy(dest);
    return compilationStatus;
}
#endif // REDE_COMPILER_IMPLEMENTATION
#if !defined(REDE_STD_H)
#define REDE_STD_H

int Rede_std(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData);

#endif // REDE_STD_H

#if defined(REDE_STD_IMPLEMENTATION)

#if !defined(REDE_STD_TABLE_H)
#define REDE_STD_TABLE_H

typedef struct RedeStdFunction {
    char* name;
    int(*call)(const RedeFunctionArgs* args, RedeVariable* result);
} RedeStdFunction;

extern size_t Rede_std_functions_size;
extern RedeStdFunction Rede_std_functions[];

#endif // REDE_STD_TABLE_H

int Rede_std_num(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_sum(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_mult(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_length(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_log(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_even(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_odd(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_incr(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_decr(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_eq(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_bool(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_not(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_less(const RedeFunctionArgs* args, RedeVariable* result);
int Rede_std_gtr(const RedeFunctionArgs* args, RedeVariable* result);

size_t Rede_std_functions_size = 42;

RedeStdFunction Rede_std_functions[42] = {
    [37] = { "num", Rede_std_num },
    [22] = { "sum", Rede_std_sum },
    [17] = { "mult", Rede_std_mult },
    [5] = { "length", Rede_std_length },
    [7] = { "log", Rede_std_log },
    [23] = { "even", Rede_std_even },
    [10] = { "odd", Rede_std_odd },
    [9] = { "incr", Rede_std_incr },
    [27] = { "decr", Rede_std_decr },
    [29] = { "eq", Rede_std_eq },
    [39] = { "bool", Rede_std_bool },
    [14] = { "not", Rede_std_not },
    [16] = { "less", Rede_std_less },
    [30] = { "gtr", Rede_std_gtr }
};

#include <string.h>

static unsigned long Rede_std_hash(const char* str) {
    unsigned long hash = 5381;
    
    char ch;
    while((ch = *(str++))) {
        hash = ((hash << 5) + hash) + ch; /* hash * 33 + c */
    }

    return hash;
}

static float Rede_std_toNumber(const RedeVariable* target) {
    switch(target->type) {
        case RedeVariableTypeNumber:
            return target->data.number;
        case RedeVariableTypeBoolean:
            return target->data.boolean;
        case RedeVariableTypeString:
            return target->data.string.length;
    }

    return 0;
}

int Rede_std_num(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setNumber(result, 0);
        return 0;
    }

    Rede_setNumber(result, Rede_std_toNumber(args->values));

    return 0;
}

int Rede_std_sum(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setNumber(result, 0);
        return 0;
    }
    float number = 0;

    for(int i = 0; i < args->length; i++) {
        number += args->values[i].data.number;
    }
    
    Rede_setNumber(result, number);

    return 0;
}

int Rede_std_mult(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setNumber(result, 0);
        return 0;
    }
    float number = 1;

    for(int i = 0; i < args->length; i++) {
        number *= args->values[i].data.number;
    }
    
    Rede_setNumber(result, number);

    return 0;
}

int Rede_std_length(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0 || args->values->type == RedeVariableTypeNumber) {
        Rede_setNumber(result, 0);
        return 0;
    }
    
    Rede_setNumber(result, (float)(args->values->data.string.length - 1));

    return 0;
}

int Rede_std_log(const RedeFunctionArgs* args, RedeVariable* result) {
    for(int i = 0; i < args->length; i++) {
        Rede_printVariable(args->values + i);
        if(i < args->length - 1) {
            printf(" ");
        }
    }
    printf("\n");

    Rede_setNumber(result, 1);

    return 0;
}

int Rede_std_even(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0 || args->values->type != RedeVariableTypeNumber) {
        Rede_setBoolean(result, 0);
    } else {
        Rede_setBoolean(result, ((int)args->values->data.number % 2) == 0);
    }

    return 0;
}

int Rede_std_odd(const RedeFunctionArgs* args, RedeVariable* result) {
    Rede_std_even(args, result);
    Rede_setBoolean(result, !result->data.boolean);

    return 0;
}

int Rede_std_incr(const RedeFunctionArgs* args, RedeVariable* result) {
    if(result->type != RedeVariableTypeNumber || args->length == 0) return 0;
    Rede_setNumber(result, args->values->data.number + 1.f);

    return 0;
}

int Rede_std_decr(const RedeFunctionArgs* args, RedeVariable* result) {
    if(result->type != RedeVariableTypeNumber || args->length == 0) return 0;
    Rede_setNumber(result, args->values->data.number - 1.f);

    return 0;
}

int Rede_std_eq(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setBoolean(result, 0);
        return 0;
    } else if(args->length == 1) {
        Rede_setBoolean(result, 1);
        return 0;
    }
    
    for(int i = 1; i < args->length; i++) {
        if(args->values[i - 1].data.number != args->values[i].data.number) {
            Rede_setBoolean(result, 0);
            return 0;
        }
    }
    
    Rede_setBoolean(result, 1);

    return 0;
}

int Rede_std_bool(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setBoolean(result, 0);
        return 0;
    }
    switch(args->values->type) {
        case RedeVariableTypeString:
            // Counting with NULL-terminator
            Rede_setBoolean(result, args->values->data.string.length > 1);
            break;
        case RedeVariableTypeNumber:
            Rede_setBoolean(result, args->values->data.number != 0);
            break;
        case RedeVariableTypeBoolean:
            Rede_setBoolean(result, args->values->data.boolean);
            break;
    }
    return 0;
}

int Rede_std_not(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setBoolean(result, 1);
        return 0;
    } else {
        Rede_std_bool(args, result);
        Rede_setBoolean(result, !result->data.boolean);
        return 0;
    }

    return 0;
}

int Rede_std_less(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length < 2) {
        Rede_setBoolean(result, 0);
        return 0;
    }

    Rede_setBoolean(result, Rede_std_toNumber(args->values) < Rede_std_toNumber(args->values + 1));

    return 0;
}

int Rede_std_gtr(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length < 2) {
        Rede_setBoolean(result, 0);
        return 0;
    }

    Rede_setBoolean(result, Rede_std_toNumber(args->values) > Rede_std_toNumber(args->values + 1));

    return 0;
}

int Rede_std(
    const char* name, size_t nameLength __attribute__ ((unused)), 
    const RedeFunctionArgs* args, RedeVariable* result, 
    void* sharedData __attribute__ ((unused))
) {
    unsigned long index = Rede_std_hash(name) % Rede_std_functions_size;

    RedeStdFunction function = Rede_std_functions[index];

    if(function.name == NULL || strcmp(function.name, name) != 0) return -2;

    return function.call(args, result);
}
#endif // REDE_STD_IMPLEMENTATION
