#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RedeCompiler.h"

// int functionCall(
//     const char* name, 
//     size_t nameLength, 
//     const RedeFunctionArgs* args, 
//     RedeVariable* result, 
//     void* sharedData
// ) {
//     float* num = sharedData;
//     if(strncmp(name, "log", nameLength) == 0) {
//         for(int i = 0; i < args->length; i++) {
//             Rede_printVariable(args->values + i);
//             if(i + 1 < args->length) {
//                 printf(", ");
//             } else {
//                 printf("\n");
//             }
//         }
//         Rede_setNumber(result, 0.0f);
//     } else if(strncmp(name, "sum", nameLength) == 0) {
//         assert(args->length == 2);
//         assert(args->values[0].type == RedeVariableTypeNumber && args->values[1].type == RedeVariableTypeNumber);
//         Rede_setNumber(result, args->values[0].data.number + args->values[1].data.number);
//     } else if(strncmp(name, "return", nameLength) == 0) {
//         assert(args->length == 1);
//         assert(args->values[0].type == RedeVariableTypeNumber);
//         *num = args->values[0].data.number;
//     } else {
//         return -1;
//     }

//     return 0;
// }

int main(void) {
char srcCode[] = \
    "first = 2\n"
    "second = 3\n"
    "c = \"hi!\"";

    RedeSource src = {
        .type = RedeSourceTypeString,
        .data = {
            .string = srcCode
        }
    };

    unsigned char buffer[100];

    CompilationMemory memory = {
        .buffer = buffer,
        .bufferLength = sizeof(buffer),
        .bufferActualLength = 0
    };

    Rede_compile(&src, &memory);

    return 0;
}