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
void Rede_printMemory(RedeRuntimeMemory*);

#endif // REDE_RUNTIME_UTILS_H

#if defined(REDE_RUNTIME_IMPLEMENTATION)
#if !defined(REDE_BYTE_CODES)
#define REDE_BYTE_CODES

#define REDE_TYPE_NUMBER 0
#define REDE_TYPE_STRING 1
#define REDE_TYPE_VAR 2
#define REDE_TYPE_STACK 3

#define REDE_CODE_ASSIGN 0
#define REDE_CODE_STACK_PUSH 1
#define REDE_CODE_CALL 2
#define REDE_CODE_STACK_CLEAR 3
#define REDE_CODE_END 255

#endif // REDE_BYTE_CODES

void Rede_setNumber(RedeVariable* variable, float number) {
    variable->type = RedeVariableTypeNumber;
    variable->data.number = number;
}

void Rede_setString(RedeVariable* variable, char* string, size_t length) {
    variable->type = RedeVariableTypeString;
    variable->data.string.string = string;
    variable->data.string.length = length;
}

void Rede_printVariable(RedeVariable* variables) {
    switch(variables->type) {
        case RedeVariableTypeNumber:
            printf("%f", variables->data.number);
            break;
        case RedeVariableTypeString:
            printf("'%s'", variables->data.string.string);
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
    memory->stringBufferActualLength -= name.data.string.length;
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
                return -1;
        }
        if(status < 0) {
            printf("Something went wrong\n");
            return -1;
        }
    }

    return 0;
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

#if defined(REDE_COMPILER_IMPLEMENTATION)
#if !defined(REDE_SOURCE_ITERATOR)
#define REDE_SOURCE_ITERATOR

typedef enum RedeSourceIteratorType {
    RedeSourceIteratorTypeString
} RedeSourceIteratorType;

typedef struct RedeSourceIterator {
    size_t index;
    int finished;
    RedeSourceIteratorType type;
    union {
        char* string;
    } data;
} RedeSourceIterator;

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator);
char RedeSourceIterator_nextChar(RedeSourceIterator* iterator);
char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index);
char RedeSourceIterator_current(RedeSourceIterator* iterator);

#endif // REDE_SOURCE_ITERATOR





#include <stdio.h>
#include <stdlib.h>

int RedeSourceIterator_init(RedeSource* src, RedeSourceIterator* iterator) {
    iterator->index = -1;
    iterator->finished = 0;
    switch(src->type) {
        case RedeSourceTypeString:
            iterator->type = RedeSourceIteratorTypeString;
            iterator->data.string = src->data.string;
            break;

        case RedeSourceTypeFile:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
            break;
    }

    return 0;
}

char RedeSourceIterator_nextChar(RedeSourceIterator* iterator) {
    if(iterator->finished) return '\0';
    iterator->index++;
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: {
            char ch = iterator->data.string[iterator->index];
            if(!ch) iterator->finished = 1;
            return ch;
        }
        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}

char RedeSourceIterator_charAt(RedeSourceIterator* iterator, size_t index) {
    switch(iterator->type) {
        case RedeSourceIteratorTypeString: 
            return iterator->data.string[index];

        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}

char RedeSourceIterator_current(RedeSourceIterator* iterator) {
    switch(iterator->type) {
        case RedeSourceIteratorTypeString:
            return iterator->data.string[iterator->index];
            
        default:
            fprintf(stderr, "File source is not implemented\n");
            exit(1);
    }
}

#if !defined(LOGS_H)
#define LOGS_H

#include <stdio.h>

#if defined(REDE_DO_LOGS)
    #define LOGS_SCOPE(name)\
        char* logs__scope__name = #name;\
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

    #define CHECK(condition, modifier, ...)\
        do {\
            int status = (condition);\
            if(status < 0) {\
                printf("LOGS '%s' Status: %d  ", logs__scope__name, status);\
                printf(__VA_ARGS__);\
                printf("\n");\
                return status + (modifier);\
            }\
        } while(0);\

    #define LOGS_ONLY(code) code

#else
    #define LOGS_SCOPE(name)
    #define LOG(...)
    #define LOG_LN(...)

    #define CHECK(condition, modifier, ...)\
        do {\
            int status = (condition);\
            if(status < 0) return status + (modifier);\
        } while(0);\

    #define LOGS_ONLY(code)

#endif // REDE_DO_LOGS


#endif // LOGS_H


#if !defined(REDE_BYTE_CODES)
#define REDE_BYTE_CODES

#define REDE_TYPE_NUMBER 0
#define REDE_TYPE_STRING 1
#define REDE_TYPE_VAR 2
#define REDE_TYPE_STACK 3

#define REDE_CODE_ASSIGN 0
#define REDE_CODE_STACK_PUSH 1
#define REDE_CODE_CALL 2
#define REDE_CODE_STACK_CLEAR 3
#define REDE_CODE_END 255

#endif // REDE_BYTE_CODES





#include <stdio.h>
#include <stdlib.h>



typedef struct RedeCompilationContext {
    int functionCallDepth;
    int isAssignment;
} RedeCompilationContext;


static int writeExpression(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeCompilationContext* ctx);

static unsigned long hash(
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

static int writeByte(RedeCompilationMemory* memory, unsigned char byte) {
    if(memory->bufferActualLength >= memory->bufferLength) return -1;

    memory->buffer[memory->bufferActualLength] = byte;
    memory->bufferActualLength++;

    return 0;
}

static size_t pow10L(size_t power) {
    size_t result = 1;

    for(size_t i = 0; i < power; i++) {
        result *= 10;
    }

    return result;
}






static int writeFloat(char firstChar, RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeCompilationContext* ctx) {
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

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);
        if(ch >= '0' && ch <= '9') {
            if(!floatingPoint) {
                result *= 10;
                result += ch - '0';
            } else {
                result += (float)(ch - '0') / pow10L(floatingPointPosition);
                floatingPointPosition++;
            }
        } else if(ch == '.' && !floatingPoint) {
            LOG_LN("Floating point");
            floatingPoint = 1;
        } else if(ch == ' ' || ch == '\n' || ch == '\r' || (ctx->functionCallDepth > 0 && ch == ')')) {
            break;
        } else {
            LOG_LN("Unexpected character");
            return -2;
        }
    }

    if(isNegative) {
        result *= -1;
    }

    LOG_LN("Result: %f", result);

    CHECK(writeByte(memory, REDE_TYPE_NUMBER), 0, "Failed to write REDE_TYPE_NUMBER");


    LOG_LN("Serializing");

    char* bytes = (char*)&result;

    for(size_t i = 0; i < sizeof(float); i++) {
        CHECK(writeByte(memory, bytes[i]), 0, "Failed to write float byte with index %zu", i);
    }

    return 0;
}






static int writeString(int singleQuoted, RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeCompilationContext* ctx) {
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

        if(endedWithQuotes) {
            LOG_LN("Did an extra iteration after the string end coz of function call");
            break;
        }

        if(isBackSlashed) {
            LOG_LN("Back-slashed");
            isBackSlashed = 0;
            pureStringLength++;
        } else if((singleQuoted && ch == '\'') || (!singleQuoted && ch == '"')) {
            LOG_LN("String end");

            endedWithQuotes = 1;


            if(ctx->functionCallDepth == 0) {
                break;
            } else {
                LOG_LN("But need to check the next char because it is function call");
                continue;
            }
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

        return -2;
    }

    LOG_LN("Chars iterated: %zu", iteratedChars);
    LOG_LN("String length: %zu", pureStringLength);


    if(pureStringLength > 255) {
        LOG_LN("String length overflow (%zu > 255)", pureStringLength);

        return -3;
    }

    CHECK(writeByte(memory, REDE_TYPE_STRING), 0, "Failed to write REDE_TYPE_STRING");

    CHECK(writeByte(memory, (unsigned char)pureStringLength), 0, "Failed to write string length");

    LOG_LN("Writing to the buffer");
    for(size_t i = stringStart; i < stringStart + iteratedChars; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(ch != '\\') {
            CHECK(writeByte(memory, (unsigned char)ch), 0, "Failed to write char");
        } 
        LOGS_ONLY(
            else {
                LOG_LN("Skiped as back-slash")
            }
        )
    }

    return 0;
}





static int writeVariableValue(
    size_t identifierStart, 
    size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory
) {
    LOGS_SCOPE(writeVariableValue);

    LOG("Identifier (s: %zu, l: %zu)", identifierStart, identifierLength);
    LOGS_ONLY(
        for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
            printf(" '%c'", RedeSourceIterator_charAt(iterator, i));
        }
        printf("\n");
    )

    unsigned long hashTableIndex = hash(iterator, identifierStart, identifierLength) % memory->variables.bufferSize;

    LOG_LN("Hash table array index: %zu", hashTableIndex);

    RedeVariableName* name = memory->variables.buffer + hashTableIndex;

    if(!name->isBusy) {
        LOG_LN("Variable is not defined");

        return -2;
    }

    LOG_LN("Variable index: %d", name->index);

    CHECK(writeByte(memory, REDE_TYPE_VAR), 0, "Failed to write REDE_TYPE_VAR");

    CHECK(writeByte(memory, name->index), 0, "Failed to write variable index");

    return 0;
}






static int writeFunctionCall(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeFunctionCall);
    ctx->functionCallDepth++;

    if(ctx->isAssignment && ctx->functionCallDepth == 1) {
        LOG_LN("Shifting the buffer cursor back because of function call inside of assignment");
        memory->bufferActualLength -= 2;
    } else if(ctx->functionCallDepth > 1) {
        LOG_LN("Shifting the buffer cursor back because of function call inside of function call");
        memory->bufferActualLength -= 1;
    }

    LOG_LN("Current function call depth: %d", ctx->functionCallDepth);

    LOG("Identifier (s: %zu, l: %zu)", identifierStart, identifierLength);
    LOGS_ONLY(
        for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
            printf(" '%c'", RedeSourceIterator_charAt(iterator, i));
        }
        printf("\n");
    )
    
    size_t argc = 0;
    while(1) {
        CHECK(writeByte(memory, REDE_CODE_STACK_PUSH), 0, "Failed to write REDE_CODE_STACK_PUSH");
        int status = writeExpression(iterator, memory, ctx);
        CHECK(status, -10, "Failed to write parameter with index %zu", argc - 1);

        if(status == 3) {
            LOG_LN("Got status 3 - just ')' at the end without expression");
            break;
        } else {
            argc++;
        }

        char argEndingChar = RedeSourceIterator_current(iterator);

        if(argEndingChar == ')') {
            LOG_LN("Got ')', end of the arguments");
            break;
        } else if(argEndingChar == ' ' || argEndingChar == '\n' || argEndingChar == '\r') {
            LOG_LN("Got white space, processing next argument");
        } else {
            LOG_LN("Unexpected end of the arguments at position '%zu'", iterator->index);

            return -2;
        }
    }
    if(ctx->functionCallDepth > 1) {
        LOG_LN("Have to check next char because of function call inside of function call");
        RedeSourceIterator_nextChar(iterator);
    }

    LOG_LN("Arguments length: %zu", argc);

    if(identifierLength > 255) {
        LOG_LN("Identifier length is too big: %zu > 255", identifierLength);
        return -1;
    }

    CHECK(writeByte(memory, REDE_CODE_CALL), 0, "Failed to write REDE_CODE_CALL");
    CHECK(writeByte(memory, (unsigned char)identifierLength), 0, "Failed to write identifier length");

    LOG_LN("Writing identifier: ");
    for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        CHECK(writeByte(memory, ch), 0, "Failed to write");
    }

    if(argc > 255) {
        LOG_LN("Too much parameters: %zu > 255", argc);
        return -1;
    }

    CHECK(writeByte(memory, (unsigned char)argc), 0, "Failed to write arguments count");

    ctx->functionCallDepth--;

    return 0;
}





static int writeFuncCOrVar(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeCompilationContext* ctx) {
    LOGS_SCOPE(writeFuncCOrVar);

    size_t identifierStart = iterator->index;
    size_t identifierLength = 1;

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(ch == ' ' || ch == '\n' || ch == '\r' || (ctx->functionCallDepth > 0 && ch == ')')) {
            LOG_LN("Variable value. ctx.functionCallDepth = %d", ctx->functionCallDepth);
            CHECK(writeVariableValue(identifierStart, identifierLength, iterator, memory), -10, "Failed to write variable value");
            return 0;
        } else if(ch == '(') {
            LOG_LN("Function call");
            CHECK(writeFunctionCall(identifierStart, identifierLength, iterator, memory, ctx), -20, "Failed to write function call");
            return 1;
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            identifierLength++;
        } else {
            LOG_LN("Unexpected char '%c'(%d)", ch, ch);

            return -1;
        }
    }

    return -1;
}





static int writeExpression(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeCompilationContext* ctx) {
    LOGS_SCOPE(writeExpression);

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);
        
        if((ch >= '0' && ch <= '9') || ch == '-') {
            LOG_LN("Number assignment");
            CHECK(writeFloat(ch, iterator, memory, ctx), -10, "Failed to write a float");
            return 0;
        } else if(ch == '"' || ch == '\'') {
            LOG_LN("String assignment");
            CHECK(writeString(ch == '\'', iterator, memory, ctx), -20, "Failed to write a string");
            return 0;
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            LOG_LN("Variable assignment or function call");
            int status = writeFuncCOrVar(iterator, memory, ctx);
            CHECK(status, -30, "Failed to write function call or variable value");
            return status;
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        } else if(ctx->functionCallDepth > 0 && ch == ')') {
            LOG_LN("GOT ')' during function arguments parsing, which means the end of the function call");
            LOG_LN("Moving cursor back by 1");
            memory->bufferActualLength--;
            
            return 3;
        } else {
            LOG_LN("Unexpected token");
            return -1;
        }
    }

    LOG_LN("Unexpected end of the string");
    return -2;
}





static int writeAssignment(
    size_t tokenStart, 
    size_t tokenLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeAssignment);
    ctx->isAssignment = 1;

    CHECK(writeByte(memory, REDE_CODE_ASSIGN), 0, "Failed to write REDE_CODE_ASSIGN to the buffer");

    unsigned long arrayIndex = hash(iterator, tokenStart, tokenLength) % memory->variables.bufferSize;
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

    CHECK(writeByte(memory, name->index), 0, "Failed to write variable index '%d' to the buffer", name->index);

    int status = writeExpression(iterator, memory, ctx);

    CHECK(status, -10, "Failed to write expression");

    if(status == 1) {
        CHECK(writeByte(memory, REDE_CODE_ASSIGN), 0, "Failed to write REDE_CODE_ASSIGN to the buffer after function call");
        CHECK(writeByte(memory, name->index), 0, "Failed to write variable index '%d' to the buffer after function call", name->index);
        CHECK(writeByte(memory, REDE_TYPE_STACK), 0, "Failed to write REDE_TYPE_STACK after function call");
    }

    ctx->isAssignment = 0;
    return 0;
}






int Rede_compile(RedeSource* src, RedeCompilationMemory* memory) {
    LOGS_SCOPE(Rede_compile);

    RedeCompilationContext ctx = {
        .isAssignment = 0,
        .functionCallDepth = 0
    };

    RedeSourceIterator iterator;
    RedeSourceIterator_init(src, &iterator);




    int searchingForTokenStart = 1;
    size_t tokenStart = 0;
    size_t tokenLength = 0;

    char ch;
    while((ch = RedeSourceIterator_nextChar(&iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            if(searchingForTokenStart) {
                tokenStart = iterator.index;
                searchingForTokenStart = 0;
            }
            tokenLength++;
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        } else if(ch == '=') {
            if(tokenLength == 0) {
                LOG_LN("Unexpected '=' literal");
                return -1;
            }
            LOG_LN("Assignment:");
            LOG("IDENTIFIER (s: %zu, l: %zu):", tokenStart, tokenLength);
            LOGS_ONLY(
                for(size_t i = tokenStart; i < tokenStart + tokenLength; i++) {
                    char ch = RedeSourceIterator_charAt(&iterator, i);
                    printf(" '%c'", ch);
                }
                printf("\n");
            )

            CHECK(writeAssignment(tokenStart, tokenLength, &iterator, memory, &ctx), -10, "Failed to write an assignment");

            searchingForTokenStart = 1;
            tokenLength = 0;
        } else if(ch == '(') {
            if(tokenLength == 0) {
                LOG_LN("Unexpected '(' literal");
                return -1;
            }
            LOG_LN("Function call:");
            CHECK(writeFunctionCall(tokenStart, tokenLength, &iterator, memory, &ctx), -100, "Failed to write function call");
            CHECK(writeByte(memory, REDE_CODE_STACK_CLEAR), 0, "Failed to write REDE_CODE_STACK_CLEAR")

            searchingForTokenStart = 1;
            tokenLength = 0;
        } else {
            LOG("Unexpected token");

            return -1;
        }
    }

    if(tokenLength > 0) {
        LOG_LN("Unexpected end of the string");
    
        return -1;
    } else {
        CHECK(writeByte(memory, REDE_CODE_END), 0, "Failed to write REDE_CODE_END");

        return 0;
    }
}
#endif // REDE_COMPILER_IMPLEMENTATION
#if !defined(REDE_STD_H)
#define REDE_STD_H

int Rede_std(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData);

#endif // REDE_STD_H

#if defined(REDE_STD_IMPLEMENTATION)

#include <string.h>

static unsigned long Rede_std_hash(const char* str) {
    unsigned long hash = 5381;
    
    char ch;
    while((ch = *(str++))) {
        hash = ((hash << 5) + hash) + ch; /* hash * 33 + c */
    }

    return hash;
}

static int Rede_std_sum(const RedeFunctionArgs* args, RedeVariable* result) {
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

static int Rede_std_mult(const RedeFunctionArgs* args, RedeVariable* result) {
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

static int Rede_std_length(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0 || args->values->type == RedeVariableTypeNumber) {
        Rede_setNumber(result, 0);
        return 0;
    }
    
    Rede_setNumber(result, (float)(args->values->data.string.length - 1));

    return 0;
}

static int Rede_std_log(const RedeFunctionArgs* args, RedeVariable* result) {
    for(int i = 0; i < args->length; i++) {
        Rede_printVariable(args->values + i);
        if(i < args->length - 1) {
            printf(", ");
        }
    }
    printf("\n");

    Rede_setNumber(result, 1);

    return 0;
}

typedef struct RedeFunction {
    char* name;
    int(*call)(const RedeFunctionArgs* args, RedeVariable* result);
} RedeFunction;

RedeFunction functions[7] = {
    [0] = { "log", Rede_std_log },
    [1] = { "sum", Rede_std_sum },
    [3] = { "mult", Rede_std_mult },
    [5] = { "length", Rede_std_length },
};

int Rede_std(
    const char* name, size_t nameLength __attribute__ ((unused)), 
    const RedeFunctionArgs* args, RedeVariable* result, 
    void* sharedData __attribute__ ((unused))
) {
    unsigned long index = Rede_std_hash(name) % (sizeof(functions) / sizeof(RedeFunction));

    RedeFunction function = functions[index];

    if(strcmp(function.name, name) != 0) return -2;

    return function.call(args, result);
}
#endif // REDE_STD_IMPLEMENTATION
