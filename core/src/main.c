#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RedeCompiler.h"
#include "RedeRuntime.h"
#include "RedeRuntimeUtils.h"
#include "RedeByteCodes.h"
#include "RedeStd.h"

int main(void) {
    // Rede_createStringSource(
    //     code,
    //     "log(length('hi!'))"
    // );

    RedeSource code = {
        .type = RedeSourceTypeFile,
        .data = {
            .path = "main.rede"
        }
    };

    Rede_createCompilationMemory(memory, 100, 256);

    int status = Rede_compile(&code, memory);

    printf("\nCode:\n");
    for(size_t i = 0; i < 100; i++) {
        printf("%d ", memory->buffer[i]);
        if(i % 32 == 0 && i != 0) {
            printf("\n");
        }
    }
    printf("\n");

    if(status < 0) return 1;

    printf("Executing the code...\n");

    Rede_createByteCodeFromBuffer(bytes, memory->buffer);

    Rede_createRuntimeMemory(runtimeMemory, 256, 256, 256);

    Rede_execute(bytes, runtimeMemory, Rede_std, NULL);

    Rede_printMemory(runtimeMemory);

    return 0;
}