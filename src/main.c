#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RedeCompiler.h"
#include "RedeRuntime.h"
#include "RedeRuntimeUtils.h"
#include "RedeByteCodes.h"

int functionCall(
    const char* name, 
    size_t nameLength, 
    const RedeFunctionArgs* args, 
    RedeVariable* result, 
    void* sharedData
) {
    float* num = sharedData;
    if(strncmp(name, "log", nameLength) == 0) {
        for(int i = 0; i < args->length; i++) {
            Rede_printVariable(args->values + i);
            if(i + 1 < args->length) {
                printf(", ");
            } else {
                printf("\n");
            }
        }
        Rede_setNumber(result, 0.0f);
    } else if(strncmp(name, "sum", nameLength) == 0) {
        assert(args->length == 2);
        assert(args->values[0].type == RedeVariableTypeNumber && args->values[1].type == RedeVariableTypeNumber);
        Rede_setNumber(result, args->values[0].data.number + args->values[1].data.number);
    } else if(strncmp(name, "random", nameLength) == 0) {
        Rede_setNumber(result, 0);
    } else if(strncmp(name, "return", nameLength) == 0) {
        assert(args->length == 1);
        assert(args->values[0].type == RedeVariableTypeNumber);
        *num = args->values[0].data.number;
    } else {
        return -1;
    }

    return 0;
}

int main(void) {
    Rede_createStringSource(
        code,
        "log("
            "sum("
                "sum(1 2)\n"
                "sum(3 4)"
            ")"
        ")"
    );

    Rede_createCompilationMemory(memory, 100, 256);

    int status = Rede_compile(code, memory);

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

    Rede_execute(bytes, runtimeMemory, functionCall, NULL);

    Rede_printMemory(runtimeMemory);

    return 0;
}