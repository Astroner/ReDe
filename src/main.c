#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Rede.h"
#include "RedeUtils.h"
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
    float result = 0;

    unsigned char buffer[] = {
        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_ASSIGN, 1, REDE_TYPE_NUMBER, 0, 0, 64, 64,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,
        REDE_CODE_CALL, 3, 's', 'u', 'm', 2,
        REDE_CODE_ASSIGN, 2, REDE_TYPE_STACK,
        REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 6, 'r', 'e', 's', 'u', 'l', 't',
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 2,
        REDE_CODE_CALL, 3, 'l', 'o', 'g', 2,
        REDE_CODE_STACK_CLEAR,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 2,
        REDE_CODE_CALL, 6, 'r', 'e', 't', 'u', 'r', 'n', 1,
        REDE_CODE_STACK_CLEAR,
        REDE_CODE_ASSIGN, 100, REDE_TYPE_STRING, 3, 'h', 'i', '!',
        REDE_CODE_END
    };

    RedeByteCode program = {
        .type = RedeByteCodeTypeBuffer,
        .data = {
            .buffer = {
                .buffer = buffer,
            }
        }
    };

    RedeVariable variables[256];
    memset(variables, 0, 256 * sizeof(RedeVariable));

    RedeVariable stack[256];
    memset(stack, 0, 256 * sizeof(RedeVariable));

    char stringBuffer[256];
    memset(stringBuffer, 0, 256);

    RedeRuntimeMemory memory = {
        .stack = stack,
        .stackActualSize = 0,
        .stackSize = 256,
        .stringBuffer = stringBuffer,
        .stringBufferActualLength = 0,
        .stringBufferLength = 256,
        .variablesBuffer = variables,
        .variablesBufferSize = 255,
    };

    Rede_execute(&program, &memory, functionCall, &result);

    printf("Got the result: %f\n", result);

    Rede_printMemory(&memory);

    return 0;
}