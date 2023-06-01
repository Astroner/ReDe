#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeDest.h"
#include "RedeCompiler.h"
#include "RedeByteCodes.h"


RedeWriteStatus RedeCompilerHelpers_writeStatement(
    RedeSourceIterator* iterator,
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeStatement);

    int lookingForTokenStart = 1;
    size_t tokenStart = 0;
    size_t tokenLength = 0;
    int tokenEnded = 0;

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("Char: '%c'(%d)", ch, ch);

        int isBracketSeparator = ctx->bracketsBlockDepth > 0 && ch == ')';

        if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            if(tokenEnded) {
                LOG_LN("Unexpected char '%c', expected function call or assignment", ch);
                return -1;
            }
            if(lookingForTokenStart) {
                lookingForTokenStart = 0;
                tokenStart = iterator->index;
                tokenLength = 0;
                tokenEnded = 0;
            }
            tokenLength++;
        } else if(
            ch == ' ' || ch == '\n' || ch == '\r' 
            ||
            isBracketSeparator
        ) {
            RedeWriteStatus status = -20;
            if(!tokenEnded && !lookingForTokenStart) {
                tokenEnded = 1;
                if(RedeCompilerHelpers_isToken("continue", tokenStart, tokenLength, iterator)) {
                    LOG_LN("Keyword: continue");

                    CHECK(status = RedeCompilerHelpers_writeContinue(dest, ctx), "Failed to write continue");
                } else if(RedeCompilerHelpers_isToken("break", tokenStart, tokenLength, iterator)) {
                    LOG_LN("Keyword: break");

                    CHECK(status = RedeCompilerHelpers_writeBreak(dest, ctx), "Failed to write break");
                } else if(RedeCompilerHelpers_isToken("while", tokenStart, tokenLength, iterator)) {
                    LOG_LN("While loop");

                    CHECK(status = RedeCompilerHelpers_writeWhile(iterator, memory, dest, ctx), "Failed to write while");
                } else if(RedeCompilerHelpers_isToken("if", tokenStart, tokenLength, iterator)) {
                    LOG_LN("If statement");

                    CHECK(status = RedeCompilerHelpers_writeIfStatement(iterator, memory, dest, ctx), "Failed to write if-statement");
                }
            }

            if(isBracketSeparator) {
                LOG_LN("Got ')' inside of brackets block. End of the block");
                return RedeWriteStatusBracketTerminated;
            }
            
            if(status != -20) {
                return status;
            }
        } else if(ch == '=' || ch == '(') {
            LOGS_ONLY(
                LOG("Token:");
                for(size_t i = tokenStart; i < tokenStart + tokenLength; i++) {
                    printf(" '%c'", RedeSourceIterator_charAt(iterator, i));
                }
                printf("\n");
            );
            if(ch == '=') {
                LOG_LN("Variable assignment");
                CHECK_RETURN(RedeCompilerHelpers_writeAssignment(tokenStart, tokenLength, iterator, memory, dest, ctx), "Failed to write assignment");
            } else {
                LOG_LN("Function call");
                CHECK(RedeCompilerHelpers_writeFunctionCall(tokenStart, tokenLength, iterator, memory, dest, ctx), "Failed to write function call");
                CHECK(RedeDest_writeByte(dest, REDE_CODE_STACK_CLEAR), "Failed to clear the stack");
                return RedeWriteStatusOk;
            }
        } else if(ch == '#') {
            LOG_LN("Comment start")
            RedeCompilerHelpers_parseComment(iterator);
            return RedeWriteStatusOk;
        } else {
            LOG_LN("Unexpected char");
            return RedeWriteStatusError;
        }
    }

    if(lookingForTokenStart && ctx->bracketsBlockDepth == 0) {
        LOG_LN("Got input end");
        return RedeWriteStatusEOI;
    }
     
    LOG_LN("Unexpected end of input");
    return RedeWriteStatusError;
}