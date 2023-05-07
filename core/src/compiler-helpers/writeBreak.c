#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeDest.h"
#include "RedeCompiler.h"
#include "RedeByteCodes.h"


int RedeCompilerHelpers_writeBreak(
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeBreak);

    if(!ctx->whileLoopCtx) {
        LOG_LN("break keyword used outside of while-loop");
        return -1;
    }
    ctx->whileLoopCtx->breakRequired = 1;

    CHECK(RedeDest_writeByte(dest, REDE_CODE_JUMP), 0, "Failed to write REDE_CODE_JUMP");
    CHECK(RedeDest_writeByte(dest, REDE_DIRECTION_BACKWARD), 0, "Failed to write REDE_DIRECTION_BACKWARD");
    
    size_t bytesDiff = dest->index - ctx->whileLoopCtx->breakJumpStart + 1;
    if(bytesDiff > 0xFFFF) {
        LOG_LN("The loop is to big to jump backward");
        return -1;
    }

    LOG_LN("Back jump length: %zu", bytesDiff);

    unsigned char* bytes = (unsigned char*)&bytesDiff;

    CHECK(RedeDest_writeByte(dest, bytes[0]), 0, "Failed to write the first byte of the back jump");
    CHECK(RedeDest_writeByte(dest, bytes[1]), 0, "Failed to write the second byte of the back jump");


    return 0;
}