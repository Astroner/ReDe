#include "RedeStd.h"

#include <assert.h>

int Rede_std(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData) {
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
        return -2;
    }

    return 0;
}