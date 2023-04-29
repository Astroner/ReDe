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
        "main.rede"
    );

    Rede_createCompilationMemory(memory, 256);

    Rede_createBufferDest(dest, 1024);

    int status = Rede_compile(code, memory, dest);

    if(status < 0) return 1;
    printf("\nCode:\n");
    if(dest->type == RedeDestTypeFile) {
        FILE* f = fopen("main.rd", "rb");

        int ch;
        while((ch = getc(f)) != EOF) {
            printf("%d ", ch);
        }

        printf("\n");
        fclose(f);
    } else {
        for(size_t i = 0; i < dest->data.buffer.length; i++) {
            printf("%d ", dest->data.buffer.buffer[i]);
        }
        printf("\n");
    }

    printf("\nExecution: \n");

    Rede_createByteCodeFromBuffer(bytes, dest->data.buffer.buffer);

    Rede_createRuntimeMemory(runtime, 256, 256, 256);

    Rede_execute(bytes, runtime, Rede_std, NULL);

    return 0;
}