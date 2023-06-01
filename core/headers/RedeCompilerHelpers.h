#include "RedeSourceIterator.h"
#include "RedeDest.h"



#if !defined(REDE_COMPILER_HELPERS)
#define REDE_COMPILER_HELPERS

/* EOI = End of input */

typedef enum RedeWriteStatus {
    RedeWriteStatusError = -1,
    RedeWriteStatusOk = 0,

    /**
     * @brief Statement ended with bracket
     * 
     */
    RedeWriteStatusBracketTerminated = 1,

    /**
     * @brief Statement ended with EOI
     * 
     */
    RedeWriteStatusEOI = 2
} RedeWriteStatus;

typedef enum RedeExpressionWriteStatus {
    RedeExpressionWriteStatusOk = 0,
    RedeExpressionWriteStatusError = -1,

    /**
     * @brief Expression is a function call
     * 
     */
    RedeExpressionWriteStatusFunction = 1,

    /**
     * @brief Expression ended with bracket
     * 
     */
    RedeExpressionWriteStatusBracketTerminated = 2,

    /**
     * @brief Expression ended with EOI
     * 
     */
    RedeExpressionWriteStatusEOI = 3,

    /**
     * @brief Got just closing bracket as brackets block end
     * 
     */
    RedeExpressionWriteStatusBracket = 4,
} RedeExpressionWriteStatus;

typedef struct RedeCompilationContextWhileLoop {
    size_t loopStart;
    size_t breakJumpStart;
    int breakRequired;
} RedeCompilationContextWhileLoop;

typedef struct RedeCompilationContext {
    int functionCallDepth;
    int isAssignment;
    int isIfStatementArgument;
    int isWhileLoopArgument;
    int bracketsBlockDepth;
    RedeCompilationContextWhileLoop* whileLoopCtx;
} RedeCompilationContext;

RedeExpressionWriteStatus RedeCompilerHelpers_writeFloat(
    char firstChar, 
    RedeSourceIterator* iterator, RedeDest* dest, RedeCompilationContext* ctx
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeString(
    int singleQuoted, 
    RedeSourceIterator* iterator, RedeDest* dest
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeVariableValue(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeOperationWithToken(
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeBoolean(int value, RedeDest* dest);

RedeExpressionWriteStatus RedeCompilerHelpers_writeExpression(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeAssignment(
    size_t tokenStart, size_t tokenLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory,
    RedeDest* dest,
    RedeCompilationContext* ctx
);

RedeExpressionWriteStatus RedeCompilerHelpers_writeFunctionCall(
    size_t identifierStart, size_t identifierLength, 
    RedeSourceIterator* iterator, 
    RedeCompilationMemory* memory, 
    RedeDest* dest,
    RedeCompilationContext* ctx
);

RedeWriteStatus RedeCompilerHelpers_writeStatements(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeStatement(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeWhile(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeContinue(RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeBreak(RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_writeIfStatement(RedeSourceIterator* iterator, RedeCompilationMemory* memory, RedeDest* dest, RedeCompilationContext* ctx);

RedeWriteStatus RedeCompilerHelpers_parseComment(RedeSourceIterator* src);


unsigned long RedeCompilerHelpers_hash(RedeSourceIterator* iterator, size_t identifierStart, size_t identifierLength);

int RedeCompilerHelpers_isToken(char* token, size_t identifierStart, size_t identifierLength, RedeSourceIterator* iterator);

#endif // REDE_COMPILER_HELPERS
