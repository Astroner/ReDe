#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




int RedeCompilerHelpers_writeFunctionCall(
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
    } else if(ctx->isWhileLoopArgument && ctx->functionCallDepth == 1) {
        LOG_LN("Shifting the buffer cursor back because of function call as while-loop argument");
        RedeDest_moveCursorBack(dest, 1);
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
        int status = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);
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