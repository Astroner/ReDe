#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




int RedeCompilerHelpers_writeIfStatement(
    RedeSourceIterator* iterator,
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeIfStatement);
    ctx->ifStatementDepth++;
    LOG_LN("Current if depth = %d", ctx->ifStatementDepth);

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), 0, "Failed to write REDE_CODE_JUMP_IF_NOT");

    int status = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);
    CHECK(status, -1, "Failed to write the condition")

    if(status == 1) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP_IF_NOT), 0, "Failed to write REDE_CODE_JUMP_IF_NOT after the function call");
        CHECK(RedeDest_writeByte(dest, REDE_TYPE_STACK), 0, "Failed to write REDE_TYPE_STACK after the function call");
    }



    LOG_LN("NOT IMPLEMENTED");
    return -1;
}