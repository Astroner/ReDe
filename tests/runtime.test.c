#include "tests-new.h"

#include "tests.h"

#include "../RedeRuntime.h"
#include "../core/headers/RedeByteCodes.h"

int funcCall(const char *name, size_t nameLength __attribute__((unused)), const RedeFunctionArgs *args, RedeVariable *result __attribute__((unused)), void *sharedData) {
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

DESCRIBE(runtime) {
    IT("executes code from buffer") {
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

        EXPECT(data) TO_BE(2.0f)
    }

    IT("executes code from file") {
        float data = 0;

        unsigned char code[] = {
            REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
            REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
            REDE_CODE_CALL, 6, 'r', 'e', 's', 'u', 'l', 't', 1,
            REDE_CODE_END
        };

        FILE* f = fopen("tests/test.rd", "wb");

        EXPECT(f) NOT TO_BE_NULL;

        fwrite(code, 1, sizeof(code), f);

        fclose(f);

        Rede_createByteCodeFromFile(bytes, "tests/test.rd");
        Rede_createRuntimeMemory(runtime, 256, 256, 256);

        EXPECT(Rede_execute(bytes, runtime, funcCall, &data)) TO_BE(0);

        EXPECT(data) TO_BE(2.0f);
    }
}
