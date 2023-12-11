#include "tests-new.h"

#include "tests.h"

#include <assert.h>
#include <stdio.h>

#include "../RedeCompiler.h"
#include "../core/headers/RedeByteCodes.h"

#define TO_BE_COMPILED_TO(BYTECODE, ...)\
        unsigned char buffer[] = { BYTECODE, __VA_ARGS__ };\
        Rede_createStringSource(src, localPassedValue);\
        Rede_createCompilationMemory(memory, 100);\
        Rede_createBufferDest(dest, 256);\
        assert(Rede_compile(src, memory, dest) == 0);\
        for(size_t i = 0; i < sizeof(buffer); i++) {\
            if(buffer[i] != dest->data.buffer.buffer[i]) {\
                printf("Got buffer mismatch at position %zu. Expected: %d, Got: %d\n", i, buffer[i], dest->data.buffer.buffer[i]);\
                TESTS_info->status = -1;\
                TESTS_info->expectText = "{ "#BYTECODE", "#__VA_ARGS__" }";\
                TESTS_info->operatorText = "TO_BE_COMPILED_TO";\
            }\
        }\
    };\


#define TO_BE_COMPILED_TO_ARR(ARR, SIZE)\
        Rede_createStringSource(src, localPassedValue);\
        Rede_createCompilationMemory(memory, 100);\
        Rede_createBufferDest(dest, 256);\
        assert(Rede_compile(src, memory, dest) == 0);\
        for(size_t i = 0; i < SIZE; i++) {\
            if(ARR[i] != dest->data.buffer.buffer[i]) {\
                printf("Got buffer mismatch at position %zu. Expected: %d, Got: %d\n", i, ARR[i], dest->data.buffer.buffer[i]);\
                TESTS_info->status = -1;\
                TESTS_info->expectText = #ARR", "#SIZE;\
                TESTS_info->operatorText = "TO_BE_COMPILED_TO";\
            }\
        }\
    };\

#define TO_HAVE_RAW_BYTES(BYTE, ...)\
        unsigned char buffer[] = { BYTE, __VA_ARGS__ };\
        for(size_t i = 0; i < sizeof(buffer); i++) {\
            if(buffer[i] != localPassedValue[i]) {\
                printf("Got buffer mismatch at position %zu. Expected: %d, Got: %d\n", i, buffer[i], localPassedValue[i]);\
                TESTS_info->status = -1;\
                TESTS_info->expectText = "{ "#BYTE", "#__VA_ARGS__" }";\
                TESTS_info->operatorText = "TO_HAVE_RAW_BYTES";\
            }\
        }\
    };\

#define FILE_TO_HAVE_CONTENT_BYTES(BYTE, ...)\
        unsigned char expect[] = { BYTE, __VA_ARGS__ };\
        FILE* f = fopen(localPassedValue, "rb");\
        assert(f != NULL);\
        size_t index = 0;\
        int el;\
        while((el = getc(f)) != EOF) {\
            if(expect[index] != el) {\
                printf("Got buffer mismatch at position %zu. Expected: %d, Got: %d\n", index, expect[index], el);\
                TESTS_info->status = -1;\
                TESTS_info->expectText = "{ "#BYTE", "#__VA_ARGS__" }";\
                TESTS_info->operatorText = "FILE_TO_HAVE_CONTENT_BYTES";\
                fclose(f);\
                return;\
            }\
            index++;\
        }\
        assert(el == EOF);\
        assert(index == sizeof(expect) / sizeof(expect[0]));\
        fclose(f);\
    };\



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
            "log( "
                "2 # Random number\n"
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
