#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




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
                LOG_LN("The if body is to big to jump forward");
                return RedeWriteStatusError;
            }

            LOG_LN("Jump size: %zu", diff);

            unsigned char* diffBytes = (unsigned char*)&diff;

            CHECK(RedeDest_writeByteAt(dest, firstJumpSizeByte + 0, diffBytes[0]), "Failed to write jump size first byte");
            CHECK(RedeDest_writeByteAt(dest, firstJumpSizeByte + 1, diffBytes[1]), "Failed to write jump size second byte");

            return resultStatus;
        }
    }


    LOG_LN("Unexpected end of the input");
    return RedeWriteStatusError;
}