#include <stdio.h>

#define REDE_RUNTIME_IMPLEMENTATION
#include "../RedeRuntime.h"

#define TEST(function)\
    printf("%s - ", #function);\
    function();\
    printf("PASS\n");\

void executesCodeFromBuffer() {
    Rede_createByteCode(
        code, 
        REDE_CODE_ASSIGN, 0, REDE_TYPE_STRING, 3, 'h', 'i', '!',
        REDE_CODE_END
    );

    Rede_createRuntimeMemory(
        memory,
        256,
        256,
        256
    );

    Rede_execute(code, memory, NULL, NULL);
}

int main(void) {
    TEST(executesCodeFromBuffer);

    return 0;
}