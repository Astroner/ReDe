#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeDest.h"
#include "RedeCompiler.h"



int RedeCompilerHelpers_writeStatements(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeStatementS);
    
    while(1) {
        int status = RedeCompilerHelpers_writeStatement(iterator, memory, dest, ctx);
        CHECK(status, 0, "Failed to write a statement");

        char currentChar = RedeSourceIterator_current(iterator);

        if(!currentChar || (ctx->whileLoopBodyDepth > 0 && currentChar == ')' && status != 1)) {
            LOGS_ONLY(
                if(ctx->whileLoopBodyDepth > 0 && currentChar == ')' && status != 1) {
                    LOG_LN("Got ')' as the end of the while-loop body");
                }
            )
            break;
        }
    }

    return 0;
}