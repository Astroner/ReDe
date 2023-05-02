#include "RedeCompilerHelpers.h"
#include "logs.h"





int RedeCompilerHelpers_writeExpression(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeExpression);

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);
        
        if((ch >= '0' && ch <= '9') || ch == '-') {
            LOG_LN("Number assignment");
            CHECK(RedeCompilerHelpers_writeFloat(ch, iterator, dest, ctx), -10, "Failed to write a float");
            return 0;
        } else if(ch == '"' || ch == '\'') {
            LOG_LN("String assignment");
            CHECK(RedeCompilerHelpers_writeString(ch == '\'', iterator, dest, ctx), -20, "Failed to write a string");
            return 0;
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            LOG_LN("Operation with token");
            int status = RedeCompilerHelpers_writeOperationWithToken(iterator, memory, dest, ctx);
            CHECK(status, -30, "Failed to write function call or variable value");
            return status;
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        } else if(ctx->functionCallDepth > 0 && ch == ')') {
            LOG_LN("GOT ')' during function arguments parsing, which means the end of the function call");
            LOG_LN("Moving cursor back by 1");
            RedeDest_moveCursorBack(dest, 1);
            
            return 3;
        } else {
            LOG_LN("Unexpected token");
            return -1;
        }
    }

    LOG_LN("Unexpected end of the string");
    return -2;
}