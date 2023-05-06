#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RedeCompiler.h"
#include "RedeRuntime.h"
#include "RedeRuntimeUtils.h"
#include "RedeByteCodes.h"
#include "RedeStd.h"

int main(void) {
    Rede_createFileSource(
        code,
        "index.rede"
    );

    Rede_createCompilationMemory(memory, 256);

    // Rede_createBufferDest(dest, 1024);
    Rede_createFileDest(dest, "index.rd");

    int status = Rede_compile(code, memory, dest);
    printf("\nCode:\n");
    if(dest->type == RedeDestTypeFile) {
        FILE* f = fopen("index.rd", "rb");

        int ch;
        while((ch = getc(f)) != EOF) {
            printf("%d ", ch);
        }

        printf("\n");
        fclose(f);
    } else {
        for(size_t i = 0; i <= dest->index; i++) {
            printf("%d ", dest->data.buffer.buffer[i]);
        }
        printf("\n");
    }
    if(status < 0) return 1;

    printf("\nExecution: \n");

    // Rede_createByteCodeFromBuffer(bytes, dest->data.buffer.buffer);
    Rede_createByteCodeFromFile(bytes, "index.rd");

    // Rede_createByteCode(bytes,
    //     REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 0,
    //     REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
    //     REDE_CODE_STACK_PUSH, REDE_TYPE_NUMBER, 0, 0, 32, 65,
    //     REDE_CODE_CALL, 2, 'e', 'q', 2,
    //     REDE_CODE_CALL, 3, 'n', 'o', 't', 1,
    //     REDE_CODE_JUMP_IF_NOT, REDE_TYPE_STACK, REDE_DIRECTION_FORWARD, 27, 0,
    //     REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
    //     REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
    //     REDE_CODE_STACK_CLEAR,
    //     REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
    //     REDE_CODE_CALL, 4, 'i', 'n', 'c', 'r', 1,
    //     REDE_CODE_ASSIGN, 0, REDE_TYPE_STACK,
    //     REDE_CODE_JUMP, REDE_DIRECTION_BACKWARD, 50, 0,
    //     REDE_CODE_END
    // )

    Rede_createRuntimeMemory(runtime, 256, 256, 256);

    Rede_execute(bytes, runtime, Rede_std, NULL);

    Rede_printMemory(runtime);

    return 0;
}