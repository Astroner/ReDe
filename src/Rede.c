#include "Rede.h"

// int Rede_compile(RedeSource* src, char* buffer, size_t bufferLength) {
    

//     return 0;
// }

void Rede_setNumber(RedeVariable* variable, float number) {
    variable->type = RedeVariableTypeNumber;
    variable->data.number = number;
}

void Rede_setString(RedeVariable* variable, char* string, size_t length) {
    variable->type = RedeVariableTypeString;
    variable->data.string.string = string;
    variable->data.string.length = length;
}

void Rede_printVariable(RedeVariable* variables) {
    switch(variables->type) {
        case RedeVariableTypeNumber:
            printf("%f", variables->data.number);
            break;
        case RedeVariableTypeString:
            printf("'%s'", variables->data.string.string);
            break;
        default:
            printf("Unknown type\n");
    }
}

void Rede_printlnVariable(RedeVariable* variables) {
    Rede_printVariable(variables);
    printf("\n");
}

static size_t setVariable(
    unsigned char* program, 
    RedeVariable* variables, 
    RedeVariable* stack, size_t* stackLength,
    RedeVariable* result
) {
    size_t length = 1;
    switch(program[0]) {
        case REDE_TYPE_NUMBER:
            result->type = RedeVariableTypeNumber;
            result->data.number = *(float*)(program + 1);
            length += 4;
            break;
        case REDE_TYPE_STRING:
            result->type = RedeVariableTypeString;
            result->data.string.length = program[1];
            result->data.string.string = (char*)program + 2;
            length += result->data.string.length + 1;
            break;
        case REDE_TYPE_VAR: {
            RedeVariable src = variables[program[1]];
            result->type = src.type;
            result->data = src.data;
            length += 1;
            break;
        }
        case REDE_TYPE_STACK: {
            if(stackLength == 0) return 0;
            *stackLength -= 1;
            RedeVariable src = stack[*stackLength];
            result->type = src.type;
            result->data = src.data;
            break;
        }
    }

    return length;
}

static size_t assignVariable(
    unsigned char* program, 
    RedeVariable* variables, 
    RedeVariable* stack, size_t* stackLength
) {
    size_t length = 2;
    RedeVariable* variable = variables + program[1];

    length += setVariable(program + 2, variables, stack, stackLength, variable);

    return length;
}

static size_t putOnStack(
    unsigned char* program, 
    RedeVariable* variables, 
    RedeVariable* stack, size_t* stackLength
) {
    size_t length = 1;

    RedeVariable* variable = stack + *stackLength;

    length += setVariable(program + 1, variables, stack, stackLength, variable);

    *stackLength += 1;

    return length;
} 

static size_t functionCall(
    unsigned char* program, 
    RedeVariable* stack, size_t* stackLength, 
    int (*funcCall)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
) {
    size_t length = 3;

    int nameLength = program[1];
    length += nameLength;

    char* name = (char*)program + 2;

    unsigned int argumentsNumber = program[length - 1];

    if(*stackLength < argumentsNumber) {
        printf("Not enough variables on the stack to call function '%s'. Actual length: %zu, Required: %d\n", name, *stackLength, argumentsNumber);
        return 0;
    }
    *stackLength -= argumentsNumber;
    RedeVariable* argumentsStart = stack + *stackLength;

    RedeFunctionArgs args = {
        .length = argumentsNumber,
        .values = argumentsStart,
    };

    RedeVariable result;

    int status = funcCall(name, nameLength, &args, &result, sharedData);

    if(status < 0) {
        return 0;
    }

    argumentsStart->type = result.type;
    argumentsStart->data = result.data;
    *stackLength += 1;

    return length;
} 

int Rede_execute(
    unsigned char* program, 
    int (*funcCall)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
) {
    RedeVariable variables[256];
    RedeVariable stack[256];
    size_t stackLength = 0;
    
    size_t index = 0;
    unsigned char code;
    while((code = program[index]) != REDE_CODE_END) {
        size_t length = 0;
        switch(code) {
            case REDE_CODE_ASSIGN:
                length = assignVariable(program + index, variables, stack, &stackLength);
                break;
            case REDE_CODE_STACK_PUSH:
                length = putOnStack(program + index, variables, stack, &stackLength);
                break;
            case REDE_CODE_CALL:
                length = functionCall(program + index, stack, &stackLength, funcCall, sharedData);
                break;
            default:
                printf("Unknown statement\n");
        }
        if(length == 0) {
            return -1;
        }
        index += length;
    }

    return 0;
}
