#if !defined(REDE_RUNTIME_PATH)
#define REDE_RUNTIME_PATH "RedeRuntime.h"
#endif // REDE_RUNTIME_PATH

#include REDE_RUNTIME_PATH

#if !defined(REDE_STD_H)
#define REDE_STD_H

int Rede_std(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData);

#endif // REDE_STD_H

#if defined(REDE_STD_IMPLEMENTATION)

#include <string.h>

static unsigned long Rede_std_hash(const char* str) {
    unsigned long hash = 5381;
    
    char ch;
    while((ch = *(str++))) {
        hash = ((hash << 5) + hash) + ch; /* hash * 33 + c */
    }

    return hash;
}

static int Rede_std_sum(const RedeFunctionArgs* args, RedeVariable* result) {
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

static int Rede_std_mult(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setNumber(result, 0);
        return 0;
    }
    float number = 1;

    for(int i = 0; i < args->length; i++) {
        number *= args->values[i].data.number;
    }
    
    Rede_setNumber(result, number);

    return 0;
}

static int Rede_std_length(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0 || args->values->type == RedeVariableTypeNumber) {
        Rede_setNumber(result, 0);
        return 0;
    }
    
    Rede_setNumber(result, (float)(args->values->data.string.length - 1));

    return 0;
}

static int Rede_std_log(const RedeFunctionArgs* args, RedeVariable* result) {
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

typedef struct RedeFunction {
    char* name;
    int(*call)(const RedeFunctionArgs* args, RedeVariable* result);
} RedeFunction;

RedeFunction functions[7] = {
    [0] = { "log", Rede_std_log },
    [1] = { "sum", Rede_std_sum },
    [3] = { "mult", Rede_std_mult },
    [5] = { "length", Rede_std_length },
};

int Rede_std(
    const char* name, size_t nameLength __attribute__ ((unused)), 
    const RedeFunctionArgs* args, RedeVariable* result, 
    void* sharedData __attribute__ ((unused))
) {
    unsigned long index = Rede_std_hash(name) % (sizeof(functions) / sizeof(RedeFunction));

    RedeFunction function = functions[index];

    if(strcmp(function.name, name) != 0) return -2;

    return function.call(args, result);
}
#endif // REDE_STD_IMPLEMENTATION
