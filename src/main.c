#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RedeCompiler.h"
#include "RedeRuntime.h"
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

// int main(void) {
//     unsigned char bytes[] = {
//         REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 3, 'h', 'i', '!',
//         REDE_CODE_CALL, 3, 'l', 'o', 'g', 1,
//         REDE_CODE_STACK_CLEAR,
//         REDE_CODE_END
//     };
//     RedeByteCode src = {
//         .type = RedeByteCodeTypeBuffer,
//         .data = {
//             .buffer = {
//                 .buffer = bytes
//             }
//         }
//     };

//     RedeVariable stack[256];
//     memset(stack, 0, sizeof(stack));

//     RedeVariable variables[256];
//     memset(variables, 0, sizeof(variables));
    
//     char strings[256];
//     memset(strings, 0, sizeof(strings));

//     RedeRuntimeMemory memory = {
//         .stack = stack,
//         .stackActualSize = 0,
//         .stackSize = 256,
//         .stringBuffer = strings,
//         .stringBufferActualLength = 0,
//         .stringBufferLength = 256,
//         .variablesBuffer = variables,
//         .variablesBufferSize = 256,
//     };

//     Rede_execute(&src, &memory, functionCall, NULL);

//     Rede_printMemory(&memory);

//     return 0;
// }
int main(void) {
    char srcCode[] = "aaa=1 aaa = 2 bb=3";

    RedeSource src = {
        .type = RedeSourceTypeString,
        .data = {
            .string = srcCode
        }
    };

    unsigned char buffer[100];
    memset(buffer, 0, sizeof(buffer));

    RedeVariableName names[256];
    memset(names, 0, sizeof(names));

    RedeCompilationMemory memory = {
        .buffer = buffer,
        .bufferLength = sizeof(buffer),
        .bufferActualLength = 0,
        .variables = {
            .buffer = names,
            .bufferSize = 256,
            .nextIndex = 0,
        }
    };

    Rede_compile(&src, &memory);

    printf("\nCode:\n");
    for(size_t i = 0; i < 100; i++) {
        printf("%d ", buffer[i]);
        if(i % 32 == 0 && i != 0) {
            printf("\n");
        }
    }
    printf("\n");


    return 0;
}