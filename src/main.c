#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Rede.h"

int functionCall(
    const char* name, 
    size_t nameLength, 
    const RedeFunctionArgs* args, 
    RedeVariable* result, 
    void* sharedData __attribute__((unused))
) {
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
    } else {
        return -1;
    }

    return 0;
}

int main(void) {
    unsigned char program[] = {
        REDE_CODE_ASSIGN, 0, REDE_TYPE_NUMBER, 0, 0, 0, 64,
        REDE_CODE_ASSIGN, 1, REDE_TYPE_NUMBER, 0, 0, 64, 64,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,
        REDE_CODE_CALL, 4, 's', 'u', 'm', '\0', 2,
        REDE_CODE_ASSIGN, 2, REDE_TYPE_STACK,
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 0,
        REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 2, '+', '\0',
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 1,
        REDE_CODE_STACK_PUSH, REDE_TYPE_STRING, 2, '=', '\0',
        REDE_CODE_STACK_PUSH, REDE_TYPE_VAR, 2,
        REDE_CODE_CALL, 4, 'l', 'o', 'g', '\0', 5,
        REDE_CODE_END
    };

    Rede_execute(program, functionCall, NULL);

    return 0;
}