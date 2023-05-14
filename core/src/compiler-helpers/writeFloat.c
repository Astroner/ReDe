#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




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