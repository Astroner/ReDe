#include "RedeUtils.h"

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

void Rede_printMemory(RedeRuntimeMemory* memory) {
    printf("Stack: (%zu/%zu)\n", memory->stackActualSize, memory->stackSize);
    for(size_t i = 0; i < memory->stackActualSize; i++) {
        printf("%zu) ", i);
        Rede_printlnVariable(memory->stack + i);
    }
    printf("\nVariables: (%d)\n", memory->variablesBufferSize);
    for(unsigned char i = 0; i < memory->variablesBufferSize; i++) {
        if(memory->variablesBuffer[i].busy) {
            printf("%d) ", i);
            Rede_printlnVariable(memory->variablesBuffer + i);
        }
    }
    printf("\nString Buffer: (%zu/%zu)\n", memory->stringBufferActualLength, memory->stringBufferLength);
    for(size_t i = 0; i < memory->stringBufferLength; i++) {
        if(i % 32 == 0 && i > 0) {
            printf("|\n|");
        } else if(i == 0) {
            printf("|");
        }
        if(i < memory->stringBufferActualLength) {
            if(memory->stringBuffer[i]) {
                printf("%c", memory->stringBuffer[i]);
            } else {
                printf(" ");
            }
        } else {
            printf(".");
        }
    }
    printf("|\n");
}