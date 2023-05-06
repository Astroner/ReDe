#include "RedeCompilerHelpers.h"
#include "logs.h"





int RedeCompilerHelpers_writeOperationWithToken(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeOperationWithToken);

    size_t identifierStart = iterator->index;
    size_t identifierLength = 1;

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(
            ch == ' ' || ch == '\n' || ch == '\r' 
            || 
            (ctx->functionCallDepth > 0 && ch == ')')
            ||
            (ctx->whileLoopBodyDepth > 0 && ch == ')')
        ) {
            LOGS_ONLY(
                if(ctx->functionCallDepth > 0 && ch == ')') {
                    LOG_LN("Token cut byt ')' token at the end of the function call");
                } else if(ctx->whileLoopBodyDepth > 0 && ch == ')') {
                    LOG_LN("Token cut byt ')' token at the end of the while-loop body");
                }
            );

            if(RedeCompilerHelpers_isToken("true", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'true'");
                CHECK(RedeCompilerHelpers_writeBoolean(1, dest), 0, "Failed to write boolean");
            } else if(RedeCompilerHelpers_isToken("false", identifierStart, identifierLength, iterator)) {
                LOG_LN("Boolean value 'false'");
                CHECK(RedeCompilerHelpers_writeBoolean(0, dest), 0, "Failed to write boolean");
            } else if(RedeCompilerHelpers_isToken("if", identifierStart, identifierLength, iterator)) {
                LOG_LN("If-statement");
                LOG_LN("NOT IMPLEMENTED");
                return -1;
            } else {
                LOG_LN("Variable value");
                CHECK(RedeCompilerHelpers_writeVariableValue(identifierStart, identifierLength, iterator, memory, dest), -10, "Failed to write variable value");
            }
            return 0;
        } else if(ch == '(') {
            LOG_LN("Function call");
            CHECK(RedeCompilerHelpers_writeFunctionCall(identifierStart, identifierLength, iterator, memory, dest, ctx), -20, "Failed to write function call");
            return 1;
        } else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            identifierLength++;
        } else {
            LOG_LN("Unexpected char '%c'(%d)", ch, ch);

            return -1;
        }
    }

    return -1;
}