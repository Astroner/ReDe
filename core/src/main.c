#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RedeCompiler.h"
#include "RedeRuntime.h"
#include "RedeRuntimeUtils.h"
#include "RedeByteCodes.h"
#include "RedeStd.h"

int main(void) {
    // Rede_createFileSource(
    //     code,
    //     "main.rede"
    // );

    // Rede_createCompilationMemory(memory, 256);

    // Rede_createBufferDest(dest, 1024);

    // int status = Rede_compile(code, memory, dest);

    // if(status < 0) return 1;
    // printf("\nCode:\n");
    // if(dest->type == RedeDestTypeFile) {
    //     FILE* f = fopen("main.rd", "rb");

    //     int ch;
    //     while((ch = getc(f)) != EOF) {
    //         printf("%d ", ch);
    //     }

    //     printf("\n");
    //     fclose(f);
    // } else {
    //     for(size_t i = 0; i < dest->data.buffer.length; i++) {
    //         printf("%d ", dest->data.buffer.buffer[i]);
    //     }
    //     printf("\n");
    // }

    printf("\nExecution: \n");

    // Rede_createByteCodeFromBuffer(bytes, dest->data.buffer.buffer);
    Rede_createByteCode(bytes, 
        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 0,

        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 32, 65,
        REDE_CODE_CALL, 2, 'e', 'q', 2, 
        
        REDE_CODE_JUMP_IF, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 59, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,

        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_CALL, 4, 'e', 'v', 'e', 'n', 1,

        REDE_CODE_JUMP_IF_NOT, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 11, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 4, 'e', 'v', 'e', 'n',
        REDE_CODE_JUMP, REDE_DIRECTION_FORWARD, 6, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 3, 'o', 'd', 'd',

        REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,
        REDE_CODE_STACK_CLEAR,

        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_CALL, 4, 'i', 'n', 'c', 'r', 1,
        REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,

        REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 76, 0,
        REDE_CODE_END
    );

    Rede_createRuntimeMemory(runtime, 256, 256, 256);

    Rede_execute(bytes, runtime, Rede_std, NULL);

    Rede_printMemory(runtime);

    return 0;
}