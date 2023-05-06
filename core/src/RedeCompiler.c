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
        .ifStatementDepth = 0,
        .isWhileLoopArgument = 0,
        .whileLoopBodyDepth = 0
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


    CHECK_ELSE(
        RedeCompilerHelpers_writeStatements(&iterator, memory, dest, &ctx),
        EXIT_COMPILER(CONDITION_VALUE),
        "Failed to parse statements"
    );


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