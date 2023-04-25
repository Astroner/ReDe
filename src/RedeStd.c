#include "RedeStd.h"

static unsigned long hash(const char* str) {
    unsigned long hash = 5381;
    
    char ch;
    while((ch = *(str++))) {
        hash = ((hash << 5) + hash) + ch; /* hash * 33 + c */
    }

    return hash;
}

static int sum(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setNumber(result, 0);
        return 0;
    }
    float number = 0;

    for(int i = 0; i < args->length; i++) {
        number += args->values[i].data.number;
    }
    
    Rede_setNumber(result, number);

    return 0;
}

static int logToConsole(const RedeFunctionArgs* args, RedeVariable* result) {
    for(int i = 0; i < args->length; i++) {
        Rede_printVariable(args->values + i);
        if(i < args->length - 1) {
            printf(", ");
        }
    }
    printf("\n");

    Rede_setNumber(result, 1);

    return 0;
}

typedef struct Function {
    int(*call)(const RedeFunctionArgs* args, RedeVariable* result);
} Function;

Function functions[] = {
    { sum },
    { logToConsole }
};

int Rede_std(
    const char* name, size_t nameLength __attribute__ ((unused)), 
    const RedeFunctionArgs* args, RedeVariable* result, 
    void* sharedData __attribute__ ((unused))
) {
    unsigned long index = hash(name) % (sizeof(functions) / sizeof(void*));

    Function* function = functions + index;

    function->call(args, result);

    return 0;
}