#include "RedeCompiler.h"
#include "RedeByteCodes.h"
#include "RedeSourceIterator.h"
#include "RedeDest.h"
#include "logs.h"
#include "RedeCompilerHelpers.h"


#include <stdio.h>
#include <stdlib.h>



#define EXIT_COMPILER(code)\
    compilationStatus = code;\
    goto exit_compiler;\

int Rede_compile(RedeSource* src, RedeCompilationMemory* memory, RedeDest* dest) {
    LOGS_SCOPE(Rede_compile);
    int compilationStatus = 0;

    RedeCompilationContext ctx = {
        .isAssignment = 0,
        .functionCallDepth = 0,
        .ifStatementDepth = 0
    };

    RedeSourceIterator iterator;
    if(RedeSourceIterator_init(src, &iterator) < 0) {
        LOG_LN("Failed to create iterator");
        return -1;
    };

    if(RedeDest_init(dest) < 0) {
        LOG_LN("Failed to init destination");
        EXIT_COMPILER(-1);
    }



    int searchingForTokenStart = 1;
    size_t tokenStart = 0;
    size_t tokenLength = 0;
    int tokenEnded = 0;

    char ch;
    while((ch = RedeSourceIterator_nextChar(&iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            if(searchingForTokenStart) {
                tokenStart = iterator.index;
                searchingForTokenStart = 0;
                tokenEnded = 0;
            }
            if(tokenEnded) {
                LOG_LN("Unexpected char '%c' at position %zu", ch, iterator.index);
                EXIT_COMPILER(-1);
            }

            tokenLength++;
        } else if(ch == ' ' || ch == '\n' || ch == '\r') {
            if(!tokenEnded && !searchingForTokenStart) {
                tokenEnded = 1;
                if(RedeCompilerHelpers_isToken("if", tokenStart, tokenLength, &iterator)) {
                    LOG_LN("If-statement");
                    CHECK_ELSE(
                        RedeCompilerHelpers_writeIfStatement(&iterator, memory, dest, &ctx), 
                        EXIT_COMPILER(CONDITION_VALUE - 200), 
                        "Failed to write if-statement"
                    );
                    searchingForTokenStart = 1;
                    tokenLength = 0;
                } else if(RedeCompilerHelpers_isToken("while", tokenStart, tokenLength, &iterator)) {
                    LOG_LN("While loop");
                    LOG_LN("Mot implemented");
                    EXIT_COMPILER(-1);
                }
            }
        } else if(ch == '=') {
            if(tokenLength == 0) {
                LOG_LN("Unexpected '=' literal");

                EXIT_COMPILER(-1);
            }
            LOG_LN("Assignment:");
            LOG("IDENTIFIER (s: %zu, l: %zu):", tokenStart, tokenLength);
            LOGS_ONLY(
                for(size_t i = tokenStart; i < tokenStart + tokenLength; i++) {
                    char ch = RedeSourceIterator_charAt(&iterator, i);
                    printf(" '%c'", ch);
                }
                printf("\n");
            )

            CHECK_ELSE(
                RedeCompilerHelpers_writeAssignment(tokenStart, tokenLength, &iterator, memory, dest, &ctx), 
                EXIT_COMPILER(CONDITION_VALUE - 10), 
                "Failed to write an assignment"
            );

            searchingForTokenStart = 1;
            tokenLength = 0;
        } else if(ch == '(') {
            if(tokenLength == 0) {
                LOG_LN("Unexpected '(' literal");

                EXIT_COMPILER(-1);
            }
            LOG_LN("Function call:");
            CHECK_ELSE(
                RedeCompilerHelpers_writeFunctionCall(tokenStart, tokenLength, &iterator, memory, dest, &ctx), 
                EXIT_COMPILER(CONDITION_VALUE - 100), 
                "Failed to write function call"
            );
            CHECK_ELSE(
                RedeDest_writeByte(dest, REDE_CODE_STACK_CLEAR), 
                EXIT_COMPILER(CONDITION_VALUE), 
                "Failed to write REDE_CODE_STACK_CLEAR"
            )

            searchingForTokenStart = 1;
            tokenLength = 0;
        } else {
            LOG("Unexpected token");

            EXIT_COMPILER(-1);
        }
    }

    if(tokenLength > 0) {
        LOG_LN("Unexpected end of the string");
    
        EXIT_COMPILER(-1);
    }

    CHECK_ELSE(
        RedeDest_writeByte(dest, REDE_CODE_END), 
        EXIT_COMPILER(CONDITION_VALUE), 
        "Failed to write REDE_CODE_END"
    );


exit_compiler:
    RedeSourceIterator_destroy(&iterator);
    RedeDest_destroy(dest);
    return compilationStatus;
}