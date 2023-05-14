#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




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