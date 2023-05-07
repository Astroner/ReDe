#include <stdio.h>
#include <assert.h>

#include "testing.h"

#define REDE_COMPILER_IMPLEMENTATION
#include "../RedeCompiler.h"

#define BUFFER_MATCH(result, ...)\
    {\
        unsigned char buffer[] = { __VA_ARGS__ };\
        for(size_t i = 0; i < sizeof(buffer); i++) {\
            if(buffer[i] != result[i]) {\
                printf("Got buffer mismatch at position %zu. Expected: %d, Got: %d\n", i, buffer[i], result[i]);\
                exit(1);\
            }\
        }\
    }\

#define MATCH(code, ...)\
    do {\
        Rede_createStringSource(src, code);\
        Rede_createCompilationMemory(memory, 100);\
        Rede_createBufferDest(dest, 256);\
        assert(Rede_compile(src, memory, dest) == 0);\
        BUFFER_MATCH(\
            dest->data.buffer.buffer,\
            __VA_ARGS__\
        );\
    } while(0);\

void compilesAssignment() {
    MATCH(
        "a = 2\n"
        "b = 'hi!'",

        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_ASSIGN, 1, REDE_TYPE_STRING, 3, 'h', 'i', '!',
        REDE_CODE_END
    );
}

void compilesFunctionCalls() {
    MATCH(
        "a = random(   )\n"
        "b = random()\n"
        "log( a   2  )",

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

void compilesFunctionCallsInsideOfFunctionCalls() {
    MATCH(
        "log(sum(2   2)   length('kekW'))",

        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_CALL, 3, 's', 'u', 'm', 2,
        REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 4, 'k', 'e', 'k', 'W',
        REDE_CODE_CALL, 6, 'l', 'e', 'n', 'g', 't', 'h', 1,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,
        REDE_CODE_STACK_CLEAR,
        REDE_CODE_END
    );
}

void compilesFromFileSource() {
    Rede_createFileSource(src, "./tests/test-code.txt");

    Rede_createCompilationMemory(memory, 100);

    Rede_createBufferDest(dest, 256);

    assert(Rede_compile(src, memory, dest) == 0);

    BUFFER_MATCH(
        dest->data.buffer.buffer,
        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_ASSIGN, 1, REDE_TYPE_STRING, 3, 'h', 'i', '!',
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,
        REDE_CODE_STACK_CLEAR,
        REDE_CODE_END
    );
}

void compilerIntoFile() {
    Rede_createStringSource(src, 
        "a = length('hi!') "
        "log(sum(a  2))"
    );

    Rede_createCompilationMemory(memory, 100);

    Rede_createFileDest(dest, "./tests/test.rd");

    assert(Rede_compile(src, memory, dest) == 0);

    int expect[] = {
        REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 3, 'h', 'i', '!',
        REDE_CODE_CALL, 6, 'l', 'e', 'n', 'g', 't', 'h', 1,
        REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_CALL, 3, 's', 'u', 'm', 2,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
        REDE_CODE_STACK_CLEAR,
        REDE_CODE_END
    };

    FILE* f = fopen("./tests/test.rd", "rb");

    assert(f != NULL);

    size_t index = 0;

    int el;
    while((el = getc(f)) != EOF) {
        if(expect[index] != el) {
            printf("Got buffer mismatch at position %zu. Expected: %d, Got: %d\n", index, expect[index], el);\
            fclose(f);
            exit(1);
        }
        index++;
    }
    assert(el == EOF);
    assert(index == sizeof(expect) / sizeof(int));

    fclose(f);
}

void compilesSimpleWhileLoops() {
    MATCH(
        "a = 0 "
        "while not(eq(a  10)) ("
            "log(a) "
            "a = incr(a)"
        ")",

        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 0,
        REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP,
        REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 32, 65,
        REDE_CODE_CALL, 2, 'e', 'q', 2,
        REDE_CODE_CALL, 3, 'n', 'o', 't', 1,
        REDE_CODE_JUMP_IF_NOT, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 27, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
        REDE_CODE_STACK_CLEAR,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_CALL, 4, 'i', 'n', 'c', 'r', 1,
        REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,
        REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 50, 0,
        REDE_CODE_END
    )
}

void compilesBreakWhileLoop() {
    MATCH(
        "while true ("
            "log(2) "
            "break"
        ")",

        REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 4, 0,
        REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 27, 0,
        REDE_CODE_JUMP_IF_NOT, REDE_TYPE_BOOL, 1, REDE_DIRECTION_FORWARD, 21, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
        REDE_CODE_STACK_CLEAR, 
        REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 25, 0,
        REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 25, 0,
        REDE_CODE_END
    )
}

void compilesContinueWhileLoop() {
    MATCH(
        "while true ("
            "continue "
            "log(2)"
        ")",
        
        REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP,
        REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP, REDE_CODE_NOP,
        REDE_CODE_JUMP_IF_NOT, REDE_TYPE_BOOL, 1, REDE_DIRECTION_FORWARD, 21, 0,
        REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 8, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
        REDE_CODE_STACK_CLEAR, 
        REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 25, 0,
        REDE_CODE_END
    )
}

int main() {
    printf("\nCompiler tests:\n");
    TEST(compilesAssignment);
    TEST(compilesFunctionCalls);
    TEST(compilesFunctionCallsInsideOfFunctionCalls);
    TEST(compilesFromFileSource);
    TEST(compilerIntoFile);
    TEST(compilesSimpleWhileLoops);
    TEST(compilesBreakWhileLoop);
    TEST(compilesContinueWhileLoop);

    printf("\n");
    return 0;
}