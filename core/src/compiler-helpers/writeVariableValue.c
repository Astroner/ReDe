#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




RedeExpressionWriteStatus RedeCompilerHelpers_writeVariableValue(
    size_t identifierStart, 
    size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest
) {
    LOGS_SCOPE(writeVariableValue);

    LOG("Identifier (s: %zu, l: %zu)", identifierStart, identifierLength);
    LOGS_ONLY(
        for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
            printf(" '%c'", RedeSourceIterator_charAt(iterator, i));
        }
        printf("\n");
    )

    unsigned long hashTableIndex = RedeCompilerHelpers_hash(iterator, identifierStart, identifierLength) % memory->variables.bufferSize;

    LOG_LN("Hash table array index: %zu", hashTableIndex);

    RedeVariableName* name = memory->variables.buffer + hashTableIndex;

    if(!name->isBusy) {
        LOG_LN("Variable is not defined");

        return RedeExpressionWriteStatusError;
    }

    LOG_LN("Variable index: %d", name->index);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_VAR), "Failed to write REDE_TYPE_VAR");

    CHECK(RedeDest_writeByte(dest, name->index), "Failed to write variable index");

    return RedeExpressionWriteStatusOk;
}