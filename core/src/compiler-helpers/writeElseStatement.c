#include "logs.h"
#include "RedeCompilerHelpers.h"
#include "RedeDest.h"
#include "RedeSourceIterator.h"
#include "RedeByteCodes.h"


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