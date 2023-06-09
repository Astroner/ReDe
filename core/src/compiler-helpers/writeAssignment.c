#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




RedeWriteStatus RedeCompilerHelpers_writeAssignment(
    size_t tokenStart, size_t tokenLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeAssignment);
    ctx->isAssignment = 1;

    CHECK(RedeDest_writeByte(dest, REDE_CODE_ASSIGN), "Failed to write REDE_CODE_ASSIGN to the buffer");

    unsigned long arrayIndex = RedeCompilerHelpers_hash(iterator, tokenStart, tokenLength) % memory->variables.bufferSize;
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

    CHECK(RedeDest_writeByte(dest, name->index), "Failed to write variable index '%d' to the buffer", name->index);

    int status = RedeCompilerHelpers_writeExpression(iterator, memory, dest, ctx);

    CHECK(status, "Failed to write expression");

    if(status == RedeExpressionWriteStatusFunction) {
        CHECK(RedeDest_writeByte(dest, REDE_CODE_ASSIGN), "Failed to write REDE_CODE_ASSIGN to the buffer after function call");
        CHECK(RedeDest_writeByte(dest, name->index), "Failed to write variable index '%d' to the buffer after function call", name->index);
        CHECK(RedeDest_writeByte(dest, REDE_TYPE_STACK), "Failed to write REDE_TYPE_STACK after function call");
    }

    ctx->isAssignment = 0;
    
    switch(status) {
        case RedeExpressionWriteStatusBracketTerminated:
            return RedeWriteStatusBracketTerminated;
        
        case RedeExpressionWriteStatusEOI:
            return RedeWriteStatusEOI;
        
        default:
            return RedeWriteStatusOk;
    }
}