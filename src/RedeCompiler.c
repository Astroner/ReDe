#include "RedeCompiler.h"
#include "RedeByteCodes.h"
#include "RedeSourceIterator.h"

#include <stdio.h>
#include <stdlib.h>

#include "logs.h"

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


static size_t pow10(size_t power) {
    size_t result = 1;

    for(size_t i = 0; i < power; i++) {
        result *= 10;
    }

    return result;
}

static int writeFloat(char firstChar, RedeSourceIterator* iterator, RedeCompilationMemory* memory) {
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
                result += (float)(ch - '0') / pow10(floatingPointPosition);
                floatingPointPosition++;
            }
        } else if(ch == '.' && !floatingPoint) {
            LOG_LN("Floating point");
            floatingPoint = 1;
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
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

static writeString(RedeSourceIterator* iterator, RedeCompilationMemory* memory) {
    LOGS_SCOPE(writeString);

    
}

static int writeExpression(RedeSourceIterator* iterator, RedeCompilationMemory* memory) {
    LOGS_SCOPE(writeExpression);

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);
        
        if((ch >= '0' && ch <= '9') || ch == '-') {
            LOG_LN("Number assignment");
            CHECK(writeFloat(ch, iterator, memory), -10, "Failed to write a float");
            return 0;
        } else if(ch == '"') {
            LOG_LN("String assignment");

            return -1;
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            LOG_LN("Variable assignment or function call");

            return -1;
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        } else {
            LOG_LN("Unexpected token");
            return -1;
        }
    }

    LOG_LN("Unexpected end of the string");
    return -2;
}

static int writeAssignment(size_t tokenStart, size_t tokenLength, RedeSourceIterator* iterator, RedeCompilationMemory* memory) {
    LOGS_SCOPE(writeAssignment);

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

    CHECK(writeExpression(iterator, memory), -10, "Failed to write expression");

    return 0;
}

int Rede_compile(RedeSource* src, RedeCompilationMemory* memory) {
    LOGS_SCOPE(Rede_compile);
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

            CHECK(writeAssignment(tokenStart, tokenLength, &iterator, memory), -10, "Failed to write an assignment");

            searchingForTokenStart = 1;
            tokenLength = 0;
        } else if(ch == '(') {
            LOG_LN("Function call:");
            return -1;
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