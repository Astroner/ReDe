#include <stdio.h>
#include <assert.h>

#include "testing.h"

#define REDE_RUNTIME_IMPLEMENTATION
#include "../RedeRuntime.h"

int funcCall(const char *name, size_t nameLength, const RedeFunctionArgs *args, RedeVariable *result, void *sharedData) {
    float* data = sharedData;
    if(strcmp(name, "result") == 0) {
        assert(args->length == 1);
        assert(args->values[0].type == RedeVariableTypeNumber);

        *data = args->values[0].data.number;
    } else {
        return -2;
    }

    return 0;
}

void executesCodeFromBuffer() {
    float data = 0;

    Rede_createByteCode(
        code, 
        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_CALL, 6, 'r', 'e', 's', 'u', 'l', 't', 1,
        REDE_CODE_END
    );

    Rede_createRuntimeMemory(
        memory,
        256,
        256,
        256
    );

    Rede_execute(code, memory, funcCall, &data);

    assert(data == 2.0f);
}

void executesCodeFromFile() {
    float data = 0;

    unsigned char code[] = {
        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_CALL, 6, 'r', 'e', 's', 'u', 'l', 't', 1,
        REDE_CODE_END
    };

    FILE* f = fopen("tests/test.rd", "wb");

    assert(f != NULL);

    fwrite(code, 1, sizeof(code), f);

    fclose(f);

    Rede_createByteCodeFromFile(bytes, "tests/test.rd");
    Rede_createRuntimeMemory(runtime, 256, 256, 256);

    assert(Rede_execute(bytes, runtime, funcCall, &data) == 0);

    assert(data == 2.0f);
}

int main(void) {
    printf("\nRuntime tests:\n");
    TEST(executesCodeFromBuffer);
    TEST(executesCodeFromFile);
    
    printf("\n");
    return 0;
}