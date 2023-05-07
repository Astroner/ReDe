#include "RedeSourceIterator.h"
#include "RedeDest.h"



#if !defined(REDE_COMPILER_HELPERS)
#define REDE_COMPILER_HELPERS

typedef struct RedeCompilationContextWhileLoop {
    size_t loopStart;
    size_t breakJumpStart;
    int breakRequired;
} RedeCompilationContextWhileLoop;

typedef struct RedeCompilationContext {
    int functionCallDepth;
    int isAssignment;
    int ifStatementDepth;
    int isWhileLoopArgument;
    int whileLoopBodyDepth;
    RedeCompilationContextWhileLoop* whileLoopCtx;
} RedeCompilationContext;

int RedeCompilerHelpers_writeFloat(
    char firstChar, 
    RedeSourceIterator* iterator, RedeDest* dest, RedeCompilationContext* ctx
);

int RedeCompilerHelpers_writeString(
    int singleQuoted, 
    RedeSourceIterator* iterator, RedeDest* dest, RedeCompilationContext* ctx
);

int RedeCompilerHelpers_writeVariableValue(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest
);

int RedeCompilerHelpers_writeExpression(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

int RedeCompilerHelpers_writeAssignment(
    size_t tokenStart, size_t tokenLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
);

int RedeCompilerHelpers_writeFunctionCall(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
);

int RedeCompilerHelpers_writeOperationWithToken(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
);

int RedeCompilerHelpers_writeIfStatement(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

unsigned long RedeCompilerHelpers_hash(RedeSourceIterator* iterator, size_t identifierStart, size_t identifierLength);

int RedeCompilerHelpers_isToken(char* token, size_t identifierStart, size_t identifierLength, RedeSourceIterator* iterator);

int RedeCompilerHelpers_writeBoolean(int value, RedeDest* dest);

int RedeCompilerHelpers_writeStatements(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

int RedeCompilerHelpers_writeStatement(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

int RedeCompilerHelpers_writeWhile(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

int RedeCompilerHelpers_writeContinue(RedeDest* dest, RedeCompilationContext* ctx);

int RedeCompilerHelpers_writeBreak(RedeDest* dest, RedeCompilationContext* ctx);

#endif // REDE_COMPILER_HELPERS
