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

    #define CHECK_ELSE(condition, elseCode, ...)\
        do {\
            int CONDITION_VALUE = (condition);\
            if(CONDITION_VALUE < 0) {\
                elseCode;\
            }\
        } while(0);

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
#define REDE_CODE_END               0xFF

#endif // REDE_BYTE_CODES




int RedeDest_init(RedeDest* dest) {
    LOGS_SCOPE(RedeDest_init);
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
    LOGS_SCOPE(RedeDest_destroy);
    if(dest->type == RedeDestTypeFile) {
        fclose(dest->data.file.fp);
    }
}

int RedeDest_writeByte(RedeDest* dest, unsigned char byte) {

    switch(dest->type) {
        case RedeDestTypeBuffer:
            if(dest->data.buffer.length != dest->data.buffer.maxLength) {
                dest->data.buffer.buffer[dest->data.buffer.length] = byte;
                dest->data.buffer.length++;
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

    switch(dest->type) {
        case RedeDestTypeBuffer:
            if(dest->data.buffer.length >= n) {
                dest->data.buffer.length -= n;
            } else {
                dest->data.buffer.length = 0;
            }
            return;
    
        case RedeDestTypeFile:
            fseek(dest->data.file.fp, -n, SEEK_CUR);
            return;
            
        default:
            return;
    }
}




#include <stdio.h>
#include <stdlib.h>



typedef struct RedeCompilationContext {
    int functionCallDepth;
    int isAssignment;
} RedeCompilationContext;


static int writeExpression(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

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


static size_t pow10L(size_t power) {
    size_t result = 1;

    for(size_t i = 0; i < power; i++) {
        result *= 10;
    }

    return result;
}






static int writeFloat(
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

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_NUMBER), 0, "Failed to write REDE_TYPE_NUMBER");


    LOG_LN("Serializing");

    char* bytes = (char*)&result;

    for(size_t i = 0; i < sizeof(float); i++) {
        CHECK(RedeDest_writeByte(dest, bytes[i]), 0, "Failed to write float byte with index %zu", i);
    }

    return 0;
}






static int writeString(
    int singleQuoted, 
    RedeSourceIterator* iterator,
    RedeDest* dest,
    RedeCompilationContext* ctx
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

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_STRING), 0, "Failed to write REDE_TYPE_STRING");

    CHECK(RedeDest_writeByte(dest, (unsigned char)pureStringLength), 0, "Failed to write string length");

    LOG_LN("Writing to the buffer");
    for(size_t i = stringStart; i < stringStart + iteratedChars; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(ch != '\\') {
            CHECK(RedeDest_writeByte(dest, (unsigned char)ch), 0, "Failed to write char");
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

    unsigned long hashTableIndex = hash(iterator, identifierStart, identifierLength) % memory->variables.bufferSize;

    LOG_LN("Hash table array index: %zu", hashTableIndex);

    RedeVariableName* name = memory->variables.buffer + hashTableIndex;

    if(!name->isBusy) {
        LOG_LN("Variable is not defined");

        return -2;
    }

    LOG_LN("Variable index: %d", name->index);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_VAR), 0, "Failed to write REDE_TYPE_VAR");

    CHECK(RedeDest_writeByte(dest, name->index), 0, "Failed to write variable index");

    return 0;
}






static int writeFunctionCall(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeFunctionCall);
    ctx->functionCallDepth++;

    if(ctx->isAssignment && ctx->functionCallDepth == 1) {
        LOG_LN("Shifting the buffer cursor back because of function call inside of assignment");
        RedeDest_moveCursorBack(dest, 2);
    } else if(ctx->functionCallDepth > 1) {
        LOG_LN("Shifting the buffer cursor back because of function call inside of function call");
        RedeDest_moveCursorBack(dest, 1);
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
        CHECK(RedeDest_writeByte(dest, REDE_CODE_STACK_PUSH), 0, "Failed to write REDE_CODE_STACK_PUSH");
        int status = writeExpression(iterator, memory, dest, ctx);
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

    CHECK(RedeDest_writeByte(dest, REDE_CODE_CALL), 0, "Failed to write REDE_CODE_CALL");
    CHECK(RedeDest_writeByte(dest, (unsigned char)identifierLength), 0, "Failed to write identifier length");

    LOG_LN("Writing identifier: ");
    for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        CHECK(RedeDest_writeByte(dest, ch), 0, "Failed to write");
    }

    if(argc > 255) {
        LOG_LN("Too much parameters: %zu > 255", argc);
        return -1;
    }

    CHECK(RedeDest_writeByte(dest, (unsigned char)argc), 0, "Failed to write arguments count");

    ctx->functionCallDepth--;

    return 0;
}



static int isToken(char* token, size_t identifierStart, size_t identifierLength, RedeSourceIterator* iterator) {
    for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
        char checkCh = token[i - identifierStart];
        if(checkCh == '\n') return 0;
        if(checkCh != RedeSourceIterator_charAt(iterator, i)) return 0;
    }

    return 1;
}

static int writeBoolean(int value, RedeDest* dest) {
    LOGS_SCOPE(writeBoolean);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_BOOL), 0, "Failed to write REDE_TYPE_BOOL");
    CHECK(RedeDest_writeByte(dest, value == 0 ? 0 : 1), 0, "Failed to write boolean value");

    return 0;
}

static int writeOperationWithToken(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeOperationWithToken);

    size_t identifierStart = iterator->index;
    size_t identifierLength = 1;

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(ch == ' ' || ch == '\n' || ch == '\r' || (ctx->functionCallDepth > 0 && ch == ')')) {
            LOG_LN("Function call depth: %d", ctx->functionCallDepth);

            if(isToken("true", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'true'");
                CHECK(writeBoolean(1, dest), 0, "Failed to write boolean");
            } else if(isToken("false", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'false'");
                CHECK(writeBoolean(0, dest), 0, "Failed to write boolean");
            } else {
                LOG_LN("Variable value");
                CHECK(writeVariableValue(identifierStart, identifierLength, iterator, memory, dest), -10, "Failed to write variable value");
            }
            return 0;
        } else if(ch == '(') {
            LOG_LN("Function call");
            CHECK(writeFunctionCall(identifierStart, identifierLength, iterator, memory, dest, ctx), -20, "Failed to write function call");
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





static int writeExpression(
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
            CHECK(writeFloat(ch, iterator, dest, ctx), -10, "Failed to write a float");
            return 0;
        } else if(ch == '"' || ch == '\'') {
            LOG_LN("String assignment");
            CHECK(writeString(ch == '\'', iterator, dest, ctx), -20, "Failed to write a string");
            return 0;
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            LOG_LN("Operation with token");
            int status = writeOperationWithToken(iterator, memory, dest, ctx);
            CHECK(status, -30, "Failed to write function call or variable value");
            return status;
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        } else if(ctx->functionCallDepth > 0 && ch == ')') {
            LOG_LN("GOT ')' during function arguments parsing, which means the end of the function call");
            LOG_LN("Moving cursor back by 1");
            RedeDest_moveCursorBack(dest, 1);
            
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
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeAssignment);
    ctx->isAssignment = 1;

    CHECK(RedeDest_writeByte(dest, REDE_CODE_ASSIGN), 0, "Failed to write REDE_CODE_ASSIGN to the buffer");

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

    CHECK(RedeDest_writeByte(dest, name->index), 0, "Failed to write variable index '%d' to the buffer", name->index);

    int status = writeExpression(iterator, memory, dest, ctx);

    CHECK(status, -10, "Failed to write expression");

    if(status == 1) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_ASSIGN), 0, "Failed to write REDE_CODE_ASSIGN to the buffer after function call");
        CHECK(RedeDest_writeByte(dest, name->index), 0, "Failed to write variable index '%d' to the buffer after function call", name->index);
        CHECK(RedeDest_writeByte(dest, REDE_TYPE_STACK), 0, "Failed to write REDE_TYPE_STACK after function call");
    }

    ctx->isAssignment = 0;
    return 0;
}




#define EXIT_COMPILER(code)\
    compilationStatus = code;\
    goto exit_compiler;\

int Rede_compile(RedeSource* src, RedeCompilationMemory* memory, RedeDest* dest) {
    LOGS_SCOPE(Rede_compile);
    int compilationStatus = 0;

    RedeCompilationContext ctx = {
        .isAssignment = 0,
        .functionCallDepth = 0
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

                EXIT_COMPILER(-1);
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

            CHECK_ELSE(
                writeAssignment(tokenStart, tokenLength, &iterator, memory, dest, &ctx), 
                EXIT_COMPILER(CONDITION_VALUE - 10), 
                "Failed to write an assignment"
            );

            searchingForTokenStart = 1;
            tokenLength = 0;
        } else if(ch == '(') {
            if(tokenLength == 0) {
                LOG_LN("Unexpected '(' literal");

                EXIT_COMPILER(-1);
            }
            LOG_LN("Function call:");
            CHECK_ELSE(
                writeFunctionCall(tokenStart, tokenLength, &iterator, memory, dest, &ctx), 
                EXIT_COMPILER(CONDITION_VALUE - 100), 
                "Failed to write function call"
            );
            CHECK_ELSE(
                RedeDest_writeByte(dest, REDE_CODE_STACK_CLEAR), 
                EXIT_COMPILER(CONDITION_VALUE), 
                "Failed to write REDE_CODE_STACK_CLEAR"
            )

            searchingForTokenStart = 1;
            tokenLength = 0;
        } else {
            LOG("Unexpected token");

            EXIT_COMPILER(-1);
        }
    }

    if(tokenLength > 0) {
        LOG_LN("Unexpected end of the string");
    
        EXIT_COMPILER(-1);
    }

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
