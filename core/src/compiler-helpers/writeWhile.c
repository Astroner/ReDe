#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeDest.h"
#include "RedeCompiler.h"
#include "RedeByteCodes.h"


int RedeCompilerHelpers_writeWhile(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeWhile);

    ctx->isWhileLoopArgument = 1;

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), 0, "Failed to write REDE_CODE_JUMP_IF_NOT");

    size_t loopStart = dest->index;

    int expressionStatus = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);
    CHECK(expressionStatus, 0, "Failed to write condition");

    ctx->isWhileLoopArgument = 0;

    if(expressionStatus == 1) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), 0, "Failed to write REDE_CODE_JUMP_IF_NOT after the function call");
        CHECK(RedeDest_writeByte(dest, REDE_TYPE_STACK), 0, "Failed to write REDE_TYPE_STACK after the function call");
    }

    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_FORWARD), 0, "Failed to write REDE_DIRECTION_FORWARD");

    CHECK(RedeDest_writeByte(dest, 0), 0, "Failed to write the first byte of jump size");
    size_t jumpSizeStart = dest->index;

    CHECK(RedeDest_writeByte(dest, 0), 0, "Failed to write the second byte of jump size");

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("Char: '%c'(%d)", ch, ch);
        if(
            (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
            ||
            ch == '('
        ) {
            if(ch == '(') {
                LOG_LN("Multiple statements");

                ctx->whileLoopBodyDepth++;
                CHECK(RedeCompilerHelpers_writeStatements(iterator, memory, dest, ctx), 0, "Failed to write while-loop multiple statements");
                ctx->whileLoopBodyDepth--;
            } else {
                LOG_LN("Single statement");
                RedeSourceIterator_moveCursorBack(iterator, 1);
                CHECK(RedeCompilerHelpers_writeStatement(iterator, memory, dest, ctx), 0, "Failed to write while-loop single statement");
            }
            
            CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP), 0, "Failed to write REDE_CODE_JUMP");
            CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_BACKWARD), 0, "Failed to write REDE_DIRECTION_BACKWARD");

            size_t bytesDiff = dest->index - loopStart + 1;
            if(bytesDiff > 0xFFFF) {
                LOG_LN("The loop is to big to jump backward");
                return -1;
            }

            LOG_LN("Back jump length: %zu", bytesDiff);

            unsigned char* bytes = (unsigned char*)&bytesDiff;

            CHECK(RedeDest_writeByte(dest, bytes[0]), 0, "Failed to write the first byte of the back jump");
            CHECK(RedeDest_writeByte(dest, bytes[1]), 0, "Failed to write the second byte of the back jump");

            bytesDiff = dest->index - jumpSizeStart - 1;

            if(bytesDiff > 0xFFFF) {
                LOG_LN("The loop is to big to jump forward");
                return -1;
            }

            bytes = (unsigned char*)&bytesDiff;

            CHECK(RedeDest_writeByteAt(dest, jumpSizeStart, bytes[0]), 0, "Failed to write the first byte of the forward jump");
            CHECK(RedeDest_writeByteAt(dest, jumpSizeStart + 1, bytes[1]), 0, "Failed to write the second byte of the forward jump");


            return 0;
        } else if(ch != ' ' && ch != '\n' && ch != '\r') {
            LOG_LN("Unexpected character");
            return -1;
        }
    }

    LOG_LN("Unexpected end of the input");

    return -1;
}