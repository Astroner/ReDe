#include "RedeCompilerHelpers.h"
#include "logs.h"





RedeExpressionWriteStatus RedeCompilerHelpers_writeOperationWithToken(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeOperationWithToken);

    size_t identifierStart = iterator->index;
    size_t identifierLength = 1;

    char ch;
    while(1) {
        ch = RedeSourceIterator_nextChar(iterator);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        int isBracketSeparator = ctx->bracketsBlockDepth > 0 && ch == ')';

        if(
            ch == ' ' || ch == '\n' || ch == '\r'  || ch == '\0'
            || 
            isBracketSeparator
        ) {
            LOGS_ONLY(
                if(isBracketSeparator) {
                    LOG_LN("Token separated by ')' token at the end of brackets block");
                } else if(ch == '\0') {
                    LOG_LN("Input end");
                }
            );

            if(RedeCompilerHelpers_isToken("true", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'true'");
                CHECK(RedeCompilerHelpers_writeBoolean(1, dest), "Failed to write boolean");
            } else if(RedeCompilerHelpers_isToken("false", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'false'");
                CHECK(RedeCompilerHelpers_writeBoolean(0, dest), "Failed to write boolean");
            } else {
                LOG_LN("Variable value");
                CHECK(RedeCompilerHelpers_writeVariableValue(identifierStart, identifierLength, iterator, memory, dest), "Failed to write variable value");
            }

            if(ch == '\0') {
                return RedeExpressionWriteStatusEOI;
            } else if(isBracketSeparator) {
                return RedeExpressionWriteStatusBracketTerminated;
            } else {
                return RedeExpressionWriteStatusOk;
            }
        } else if(ch == '(') {
            LOG_LN("Function call");
            CHECK_RETURN(RedeCompilerHelpers_writeFunctionCall(identifierStart, identifierLength, iterator, memory, dest, ctx), "Failed to write function call");
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            identifierLength++;
        } else {
            LOG_LN("Unexpected char");

            return RedeExpressionWriteStatusError;
        }
    }

    LOG_LN("Unexpected loop break");
    return RedeExpressionWriteStatusError;
}