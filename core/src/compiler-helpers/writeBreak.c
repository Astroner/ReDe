#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeDest.h"
#include "RedeCompiler.h"
#include "RedeByteCodes.h"


RedeWriteStatus RedeCompilerHelpers_writeBreak(
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeBreak);

    if(!ctx->whileLoopCtx) {
        LOG_LN("break keyword used outside of while-loop");
        return RedeWriteStatusError;
    }
    ctx->whileLoopCtx->breakRequired = 1;

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP), "Failed to write REDE_CODE_JUMP");
    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_BACKWARD), "Failed to write REDE_DIRECTION_BACKWARD");
    
    size_t bytesDiff = dest->index - ctx->whileLoopCtx->breakJumpStart + 1;
    if(bytesDiff > 0xFFFF) {
        LOG_LN("The loop is to big to jump backward");
        return RedeWriteStatusError;
    }

    LOG_LN("Back jump length: %zu", bytesDiff);

    unsigned char* bytes = (unsigned char*)&bytesDiff;

    CHECK(RedeDest_writeByte(dest, bytes[0]), "Failed to write the first byte of the back jump");
    CHECK(RedeDest_writeByte(dest, bytes[1]), "Failed to write the second byte of the back jump");


    return RedeWriteStatusOk;
}