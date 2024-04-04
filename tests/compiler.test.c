#include "tests-new.h"

#include "tests.h"

#include <assert.h>
#include <stdio.h>

#include "../RedeCompiler.h"
#include "../core/headers/RedeByteCodes.h"

#define TO_BE_COMPILED_TO(BYTECODE, ...)\
    CREATE_MATCHER(TO_BE_COMPILED_TO,\
        unsigned char buffer[] = { BYTECODE, __VA_ARGS__ };\
        Rede_createStringSource(src, MATCHER_VALUE);\
        Rede_createCompilationMemory(memory, 100);\
        Rede_createBufferDest(dest, 256);\
        int status = Rede_compile(src, memory, dest);\
        if(status != 0) {\
            MATCHER_INFO("Failed to compile: %d", status);\
            MATCHER_FAIL(NO_EXPECTED);\
        }\
        for(size_t i = 0; i < sizeof(buffer); i++) {\
            MATCHER_CONDITION(PASSES_IF(buffer[i] == dest->data.buffer.buffer[i])) {\
                MATCHER_INFO("Got buffer mismatch at position %zu. Expected: %d, Got: %d", i, buffer[i], dest->data.buffer.buffer[i]);\
                MATCHER_FAIL(EXPECTED("{ "#BYTECODE", "#__VA_ARGS__" }"));\
            }\
        }\
    )


#define TO_BE_COMPILED_TO_ARR(ARR, SIZE)\
    CREATE_MATCHER(TO_BE_COMPILED_TO_ARR,\
        Rede_createStringSource(src, MATCHER_VALUE);\
        Rede_createCompilationMemory(memory, 100);\
        Rede_createBufferDest(dest, 256);\
        int status = Rede_compile(src, memory, dest);\
        if(status != 0) {\
            MATCHER_INFO("Failed to compile: %d", status);\
            MATCHER_FAIL(NO_EXPECTED);\
        }\
        for(size_t i = 0; i < SIZE; i++) {\
            MATCHER_CONDITION(PASSES_IF(ARR[i] == dest->data.buffer.buffer[i])) {\
                MATCHER_INFO("Got buffer mismatch at position %zu. Expected: %d, Got: %d", i, ARR[i], dest->data.buffer.buffer[i]);\
                MATCHER_FAIL(EXPECTED(#ARR", "#SIZE));\
            }\
        }\
    )

#define FILE_TO_HAVE_CONTENT_BYTES(BYTE, ...)\
    CREATE_MATCHER(FILE_TO_HAVE_CONTENT_BYTES, \
        unsigned char expect[] = { BYTE, __VA_ARGS__ };\
        FILE* f = fopen(MATCHER_VALUE, "rb");\
        MATCHER_CONDITION(FAILS_IF(f == NULL)) {\
            MATCHER_INFO("Failed to open %s", MATCHER_VALUE);\
            MATCHER_FAIL(NO_EXPECTED);\
        }\
        size_t index = 0;\
        int el;\
        while((el = getc(f)) != EOF) {\
            MATCHER_CONDITION(PASSES_IF(expect[index] == el)) {\
                MATCHER_INFO("Got buffer mismatch at position %zu. Expected: %d, Got: %d", index, expect[index], el);\
                fclose(f);\
                MATCHER_FAIL("{ "#BYTE", "#__VA_ARGS__" }");\
            }\
            index++;\
        }\
        MATCHER_CONDITION(PASSES_IF(el == EOF)) {\
            MATCHER_INFO("Could not reach EOF");\
            fclose(f);\
            MATCHER_FAIL(NO_EXPECTED);\
        }\
        MATCHER_CONDITION(PASSES_IF(index == sizeof(expect) / sizeof(expect[0]))) {\
            MATCHER_INFO("Could not reach end of the buffer");\
            fclose(f);\
            MATCHER_FAIL(NO_EXPECTED);\
        }\
        fclose(f);\
    )



DESCRIBE(compiler) {
    IT("compiles assignment") {
        EXPECT(
            "a = 2\n"
            "b = 'hi!'"
        ) TO_BE_COMPILED_TO(
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_ASSIGN, 1, REDE_TYPE_STRING, 3, 'h', 'i', '!',
            REDE_CODE_END
        )
    }

    IT("compiles function calls") {
        EXPECT(
            "a = random(   )\n"
            "b = random()\n"
            "log( a   2  )"
        ) TO_BE_COMPILED_TO(
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

    IT("compiles function calls inside of function calls") {
        EXPECT("log(sum(2   2)   length('kekW'))") TO_BE_COMPILED_TO(
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
    
    IT("compiles from file source") {
        Rede_createFileSource(src, "./tests/test-code.txt");

        Rede_createCompilationMemory(memory, 100);

        Rede_createBufferDest(dest, 256);

        assert(Rede_compile(src, memory, dest) == 0);

        EXPECT(dest->data.buffer.buffer) TO_HAVE_RAW_BYTES(
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_ASSIGN, 1, REDE_TYPE_STRING, 3, 'h', 'i', '!',
            REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
            REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,
            REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,
            REDE_CODE_STACK_CLEAR,
            REDE_CODE_END
        )
    }
    
    IT("compiles into file") {
        Rede_createStringSource(src, 
            "a = length('hi!') "
            "log(sum(a  2))"
        );

        Rede_createCompilationMemory(memory, 100);

        Rede_createFileDest(dest, "./tests/test.rd");

        assert(Rede_compile(src, memory, dest) == 0);

        EXPECT("./tests/test.rd") FILE_TO_HAVE_CONTENT_BYTES(
            REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 3, 'h', 'i', '!',
            REDE_CODE_CALL, 6, 'l', 'e', 'n', 'g', 't', 'h', 1,
            REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,
            REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
            REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_CALL, 3, 's', 'u', 'm', 2,
            REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
            REDE_CODE_STACK_CLEAR,
            REDE_CODE_END
        )
    }

    IT("compiles simple while loops") {
        EXPECT(
            "a = 0 "
            "while not(eq(a  10)) ("
                "log(a) "
                "a = incr(a)"
            ")"
        ) TO_BE_COMPILED_TO(
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

    IT("compiles while loops with break") {
        EXPECT(
            "while true ("
                "log(2) "
                "break"
            ")"
        ) TO_BE_COMPILED_TO(
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

    IT("compiles while loops with continue") {
        EXPECT(
            "while true ("
                "continue "
                "log(2)"
            ")"
        ) TO_BE_COMPILED_TO(            
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

    IT("compiles simple if case") {
        EXPECT("if true log(2)") TO_BE_COMPILED_TO(            
            REDE_CODE_JUMP_IF_NOT, REDE_TYPE_BOOL, 1, REDE_DIRECTION_FORWARD, 13, 0,
            REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
            REDE_CODE_STACK_CLEAR,
            REDE_CODE_END
        )
    }

    IT("compiles if-else statements") {
        EXPECT(
            "if true a = 2 "
            "else a = 3"
        ) TO_BE_COMPILED_TO(
            REDE_CODE_JUMP_IF_NOT, REDE_TYPE_BOOL, 1, REDE_DIRECTION_FORWARD, 11, 0,
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 7, 0,
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 64, 64,
            REDE_CODE_END
        )
    }

    IT("compiles if-else-if") {
        EXPECT(
            "if true a = 2 "
            "else if true a = 2 "
            "else if true a = 2 "
            "else a = 2"
        ) TO_BE_COMPILED_TO(
            REDE_CODE_JUMP_IF_NOT, REDE_TYPE_BOOL, 1, REDE_DIRECTION_FORWARD, 11, 0,
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 41, 0,
            REDE_CODE_JUMP_IF_NOT, REDE_TYPE_BOOL, 1, REDE_DIRECTION_FORWARD, 11, 0,
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 24, 0,
            REDE_CODE_JUMP_IF_NOT, REDE_TYPE_BOOL, 1, REDE_DIRECTION_FORWARD, 11, 0,
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 7, 0,
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_END
        )
    }

    IT("parses comments") {
        unsigned char expectedEmpty[] = { REDE_CODE_END };
        EXPECT("# comment") TO_BE_COMPILED_TO_ARR(expectedEmpty, sizeof(expectedEmpty));

        EXPECT(
            "# comment\n"
            "a = 2"
        ) TO_BE_COMPILED_TO(
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_END
        )

        EXPECT("a = 2 # comment") TO_BE_COMPILED_TO(
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_END
        )

        EXPECT(
            "log #Function to log# ( "
                "# Random number # 2 \n"
                "3 # Second random number\n"
                "# Arguments end\n"
            ")"
        ) TO_BE_COMPILED_TO(
            REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 64, 64,
            REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,
            REDE_CODE_STACK_CLEAR,
            REDE_CODE_END
        )
    }
}
