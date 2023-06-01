#include "RedeCompilerHelpers.h"
#include "logs.h"





RedeExpressionWriteStatus RedeCompilerHelpers_writeExpression(
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
            CHECK_RETURN(RedeCompilerHelpers_writeFloat(ch, iterator, dest, ctx), "Failed to write a float");
        } else if(ch == '"' || ch == '\'') {
            LOG_LN("String assignment");
            CHECK_RETURN(RedeCompilerHelpers_writeString(ch == '\'', iterator, dest), "Failed to write a string");
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            LOG_LN("Operation with token");
            CHECK_RETURN(RedeCompilerHelpers_writeOperationWithToken(iterator, memory, dest, ctx), "Failed to write function call or variable value");
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        } else if(ctx->bracketsBlockDepth > 0 && ch == ')') {
            LOG_LN("Brackets block end");
            
            return RedeExpressionWriteStatusBracket;
        } else if (ch == '#') {
            LOG_LN("Comment start");
            RedeCompilerHelpers_parseComment(iterator);
        } else {
            LOG_LN("Unexpected token");
            return RedeExpressionWriteStatusError;
        }
    }

    LOG_LN("Unexpected end of input");
    return RedeExpressionWriteStatusError;
}