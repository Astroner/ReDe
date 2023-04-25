#include <stdio.h>
#include <assert.h>

#include "testing.h"

#define REDE_COMPILER_IMPLEMENTATION
#include "../RedeCompiler.h"

#define MATCH(result, ...)\
    {\
        unsigned char buffer[] = { __VA_ARGS__ };\
        for(size_t i = 0; i < sizeof(buffer); i++) {\
            if(buffer[i] != result[i]) {\
                printf("Got buffer mismatch at position %zu. Expected: %d, Got: %d\nå", i, buffer[i], result[i]);\
                exit(1);\
            }\
        }\
    }\

void compilesAssignment() {
    Rede_createStringSource(src,
        "a = 2\n"
        "b = 'hi!'"
    );

    Rede_createCompilationMemory(memory, 256, 100);

    assert(Rede_compile(src, memory) == 0);

    MATCH(
        memory->buffer,
        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_ASSIGN, 1, REDE_TYPE_STRING, 3, 'h', 'i', '!',
        REDE_CODE_END
    );
}

void compilesFunctionCalls() {
    Rede_createStringSource(src,
        "a = random(   )\n"
        "b = random()\n"
        "log( a   2  )"
    );

    Rede_createCompilationMemory(memory, 256, 100);

    assert(Rede_compile(src, memory) == 0);

    MATCH(
        memory->buffer,
        REDE_CODE_CALL, 6, 'r', 'a', 'n', 'd', 'o', 'm', 0,
        REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,
        REDE_CODE_CALL, 6, 'r', 'a', 'n', 'd', 'o', 'm', 0,
        REDE_CODE_ASSIGN, 1, REDE_TYPE_STACK,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,
        REDE_CODE_STACK_CLEAR,
        REDE_CODE_END
    );
}

int main() {
    printf("\nCompiler tests:\n");
    TEST(compilesAssignment);
    TEST(compilesFunctionCalls);

    printf("\n");
    return 0;
}