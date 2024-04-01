#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeDest.h"
#include "RedeCompiler.h"


// Iterate over the source and write statements one by one
RedeWriteStatus RedeCompilerHelpers_writeStatements(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeStatementS);
    
    while(1) {
        int status = RedeCompilerHelpers_writeStatement(iterator, memory, dest, ctx);
        CHECK(status, "Failed to write a statement");

        // We do essentials the same but with different statement terminators
        if(status == RedeWriteStatusBracketTerminated) {
            LOG_LN("Last statement was bracket terminated");
            return RedeWriteStatusOk;
        } else if(status == RedeWriteStatusEOI) {
            LOG_LN("Last statement ended with EOI");
            return RedeWriteStatusOk;
        }
    }

    return RedeWriteStatusOk;
}