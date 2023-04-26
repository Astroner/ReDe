#include <stdio.h>
#include <string.h>

#include "Rede.h"

#define MAX_LINE_WIDTH 100

typedef struct CLIData {
    RedeRuntimeMemory* memory;
    int quit;
} CLIData;

int fCall(const char *name, size_t nameLength, const RedeFunctionArgs *args, RedeVariable *result, void *sharedData) {
    int status = Rede_std(name, nameLength, args, result, NULL);
    if(status != -2) return status;

    CLIData* data = sharedData;

    if(strcmp(name, "memory") == 0) {
        Rede_printMemory(data->memory);
        return 0;
    } else if(strcmp(name, "quit") == 0 || strcmp(name, "q") == 0) {
        data->quit = 1;
        return 0;
    }

    return 0;
}

int main(void) {
    printf("ReDe Scripting CLI\n");

    char buffer[MAX_LINE_WIDTH];
    memset(buffer, 0, MAX_LINE_WIDTH);

    Rede_createStringSource(code, buffer);
    Rede_createCompilationMemory(compilation, 200, 256);

    Rede_createRuntimeMemory(runtime, 100, 256, 1024);
    Rede_createByteCodeFromBuffer(bytes, compilation->buffer);

    CLIData data = {
        .quit = 0,
        .memory = runtime,
    };

    while(!data.quit) {
        fgets(buffer, MAX_LINE_WIDTH, stdin);

        int status = Rede_compile(code, compilation);

        if(status < 0) {
            printf("Failed to compile\n");
            goto loop_end;
        }

        status = Rede_execute(bytes, runtime, fCall, &data);

        if(status < 0) {
            printf("Failed to execute\n");
        }
    
loop_end:
        memset(buffer, 0, MAX_LINE_WIDTH);
        compilation->bufferActualLength = 0;
    }

    return 0;
}