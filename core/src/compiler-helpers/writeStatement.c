#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeDest.h"
#include "RedeCompiler.h"
#include "RedeByteCodes.h"


int RedeCompilerHelpers_writeStatement(
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
        } else if(ch == ' ' || ch == '\n' || ch == '\r' || (ctx->whileLoopBodyDepth > 0 && ch == ')')) {
            int writtenStatement = 0;
            if(!tokenEnded && !lookingForTokenStart) {
                tokenEnded = 1;
                if(RedeCompilerHelpers_isToken("while", tokenStart, tokenLength, iterator)) {
                    LOG_LN("While loop");
                    CHECK(RedeCompilerHelpers_writeWhile(iterator, memory, dest, ctx), 0, "Failed to write while");
                    writtenStatement = 1;
                } else if(RedeCompilerHelpers_isToken("continue", tokenStart, tokenLength, iterator)) {
                    LOG_LN("Keyword: continue");
                    CHECK(RedeCompilerHelpers_writeContinue(dest, ctx), 0, "Failed to write continue");
                    writtenStatement = 1;
                } else if(RedeCompilerHelpers_isToken("break", tokenStart, tokenLength, iterator)) {
                    LOG_LN("Keyword: break");
                    CHECK(RedeCompilerHelpers_writeBreak(dest, ctx), 0, "Failed to write break");
                    writtenStatement = 1;
                }
            }
            if(ctx->whileLoopBodyDepth > 0 && ch == ')') {
                LOG_LN("Got ')' inside of while-loop body. End of the loop");
                return 0;
            }
            if(writtenStatement) {
                return 0;
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
                int status = RedeCompilerHelpers_writeAssignment(tokenStart, tokenLength, iterator, memory, dest, ctx);
                CHECK(status, 0, "Failed to write assignment");
                return status;
            } else {
                LOG_LN("Function call");
                CHECK(RedeCompilerHelpers_writeFunctionCall(tokenStart, tokenLength, iterator, memory, dest, ctx), 0, "Failed to write function call");
                CHECK(RedeDest_writeByte(dest, REDE_CODE_STACK_CLEAR), 0, "Failed to clear the stack");
                return 1;
            }
        } else {
            LOG_LN("Unexpected char '%c'", ch);
            return -1;
        }
    }

    if(lookingForTokenStart) {
        LOG_LN("Got input end");
        return 0;
    }
     
    LOG_LN("Unexpected end of input");
    return -1;
}