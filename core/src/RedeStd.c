#include "RedeStd.h"
#include "RedeStdTable.h"

#include <string.h>

static unsigned long Rede_std_hash(const char* str) {
    unsigned long hash = 5381;
    
    char ch;
    while((ch = *(str++))) {
        hash = ((hash << 5) + hash) + ch; /* hash * 33 + c */
    }

    return hash;
}

static float Rede_std_toNumber(const RedeVariable* target) {
    switch(target->type) {
        case RedeVariableTypeNumber:
            return target->data.number;
        case RedeVariableTypeBoolean:
            return target->data.boolean;
        case RedeVariableTypeString:
            return target->data.string.length;
    }

    return 0;
}

int Rede_std_num(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setNumber(result, 0);
        return 0;
    }

    Rede_setNumber(result, Rede_std_toNumber(args->values));

    return 0;
}

int Rede_std_sum(const RedeFunctionArgs* args, RedeVariable* result) {
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

int Rede_std_mult(const RedeFunctionArgs* args, RedeVariable* result) {
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

int Rede_std_length(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0 || args->values->type == RedeVariableTypeNumber) {
        Rede_setNumber(result, 0);
        return 0;
    }
    
    Rede_setNumber(result, (float)(args->values->data.string.length - 1));

    return 0;
}

int Rede_std_log(const RedeFunctionArgs* args, RedeVariable* result) {
    for(int i = 0; i < args->length; i++) {
        Rede_printVariable(args->values + i);
        if(i < args->length - 1) {
            printf(" ");
        }
    }
    printf("\n");

    Rede_setNumber(result, 1);

    return 0;
}

int Rede_std_even(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0 || args->values->type != RedeVariableTypeNumber) {
        Rede_setBoolean(result, 0);
    } else {
        Rede_setBoolean(result, ((int)args->values->data.number % 2) == 0);
    }

    return 0;
}

int Rede_std_odd(const RedeFunctionArgs* args, RedeVariable* result) {
    Rede_std_even(args, result);
    Rede_setBoolean(result, !result->data.boolean);

    return 0;
}

int Rede_std_incr(const RedeFunctionArgs* args, RedeVariable* result) {
    if(result->type != RedeVariableTypeNumber || args->length == 0) return 0;
    Rede_setNumber(result, args->values->data.number + 1.f);

    return 0;
}

int Rede_std_decr(const RedeFunctionArgs* args, RedeVariable* result) {
    if(result->type != RedeVariableTypeNumber || args->length == 0) return 0;
    Rede_setNumber(result, args->values->data.number - 1.f);

    return 0;
}

int Rede_std_eq(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setBoolean(result, 0);
        return 0;
    } else if(args->length == 1) {
        Rede_setBoolean(result, 1);
        return 0;
    }
    
    for(int i = 1; i < args->length; i++) {
        if(args->values[i - 1].data.number != args->values[i].data.number) {
            Rede_setBoolean(result, 0);
            return 0;
        }
    }
    
    Rede_setBoolean(result, 1);

    return 0;
}

int Rede_std_bool(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setBoolean(result, 0);
        return 0;
    }
    switch(args->values->type) {
        case RedeVariableTypeString:
            // Counting with NULL-terminator
            Rede_setBoolean(result, args->values->data.string.length > 1);
            break;
        case RedeVariableTypeNumber:
            Rede_setBoolean(result, args->values->data.number != 0);
            break;
        case RedeVariableTypeBoolean:
            Rede_setBoolean(result, args->values->data.boolean);
            break;
    }
    return 0;
}

int Rede_std_not(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length == 0) {
        Rede_setBoolean(result, 1);
        return 0;
    } else {
        Rede_std_bool(args, result);
        Rede_setBoolean(result, !result->data.boolean);
        return 0;
    }

    return 0;
}

int Rede_std_less(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length < 2) {
        Rede_setBoolean(result, 0);
        return 0;
    }

    Rede_setBoolean(result, Rede_std_toNumber(args->values) < Rede_std_toNumber(args->values + 1));

    return 0;
}

int Rede_std_gtr(const RedeFunctionArgs* args, RedeVariable* result) {
    if(args->length < 2) {
        Rede_setBoolean(result, 0);
        return 0;
    }

    Rede_setBoolean(result, Rede_std_toNumber(args->values) > Rede_std_toNumber(args->values + 1));

    return 0;
}

int Rede_std(
    const char* name, size_t nameLength __attribute__ ((unused)), 
    const RedeFunctionArgs* args, RedeVariable* result, 
    void* sharedData __attribute__ ((unused))
) {
    unsigned long index = Rede_std_hash(name) % Rede_std_functions_size;

    RedeStdFunction function = Rede_std_functions[index];

    if(function.name == NULL || strcmp(function.name, name) != 0) return -2;

    return function.call(args, result);
}