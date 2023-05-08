#include "RedeRuntimeUtils.h"
#include "RedeByteIterator.h"
#include "RedeByteCodes.h"


void Rede_setNumber(RedeVariable* variable, float number) {
    variable->type = RedeVariableTypeNumber;
    variable->data.number = number;
}

void Rede_setString(RedeVariable* variable, char* string, size_t length) {
    variable->type = RedeVariableTypeString;
    variable->data.string.string = string;
    variable->data.string.length = length;
}

void Rede_setBoolean(RedeVariable* variable, int value) {
    variable->type = RedeVariableTypeBoolean;
    variable->data.boolean = value == 0 ? 0 : 1;
}

void Rede_printVariable(RedeVariable* variables) {
    switch(variables->type) {
        case RedeVariableTypeNumber:
            printf("%f", variables->data.number);
            break;
        case RedeVariableTypeString:
            printf("'%s'", variables->data.string.string);
            break;
        case RedeVariableTypeBoolean:
            printf(variables->data.boolean ? "true" : "false");
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
    printf("\nVariables: (%zu)\n", memory->variablesBufferSize);
    for(size_t i = 0; i < memory->variablesBufferSize; i++) {
        if(memory->variablesBuffer[i].busy) {
            printf("%zu) ", i);
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




int Rede_printByteCode_type(RedeByteIterator* iterator) {
    int variableType = RedeByteIterator_nextByte(iterator);

    switch(variableType) {
        case REDE_TYPE_BOOL: 
            printf(RedeByteIterator_nextByte(iterator) > 0 ? "true" : "false");
            break;

        case REDE_TYPE_NUMBER: {
            float number = 0;
            unsigned char* bytes = (unsigned char*)&number;
            bytes[0] = RedeByteIterator_nextByte(iterator);
            bytes[1] = RedeByteIterator_nextByte(iterator);
            bytes[2] = RedeByteIterator_nextByte(iterator);
            bytes[3] = RedeByteIterator_nextByte(iterator);

            printf("%f", number);
            break;
        }

        case REDE_TYPE_STACK: 
            printf("Stack value");
            break;
        
        case REDE_TYPE_STRING:
            printf("\'");
            for(int i = 0; i < RedeByteIterator_nextByte(iterator); i++) {
                printf("%c", RedeByteIterator_nextByte(iterator));
            }
            printf("\'");
            break;
        
        case REDE_TYPE_VAR:
            printf("Variable '%d'", RedeByteIterator_nextByte(iterator));

            break;
        
        default:
            printf("Unknown type %d\n", variableType);
            return -1;

    }

    return 0;
}

int Rede_printByteCode_assignment(RedeByteIterator* iterator) {
    printf("Assign to '%d' value ", RedeByteIterator_nextByte(iterator));
    Rede_printByteCode_type(iterator);
    printf("\n");

    return 0;
}

int Rede_printByteCode_call(RedeByteIterator* iterator) {
    printf("Call function '");
    int nameLength = RedeByteIterator_nextByte(iterator);
    for(int i = 0; i < nameLength; i++) {
        printf("%c", (char)RedeByteIterator_nextByte(iterator));
    }
    printf("' with arguments count %d\n", RedeByteIterator_nextByte(iterator));

    return 0;
}

int Rede_printByteCode_parseJumpSize(RedeByteIterator* iterator) {
    int jumpSize = 0;
    unsigned char* jumpBytes = (unsigned char*)&jumpSize;
    jumpBytes[0] = RedeByteIterator_nextByte(iterator);
    jumpBytes[1] = RedeByteIterator_nextByte(iterator);

    return jumpSize;
}

int Rede_printByteCode_jump(RedeByteIterator* iterator) {
    printf(RedeByteIterator_nextByte(iterator) == 0 ? "Jump forward " : "Jump backward ");

    int jump = Rede_printByteCode_parseJumpSize(iterator);

    printf("%d bytes\n", jump);

    return 0;
}

int Rede_printByteCode_jumpIf(RedeByteIterator* iterator) {
    printf("If ");
    Rede_printByteCode_type(iterator);
    printf(RedeByteIterator_nextByte(iterator) == 0 ? " jump forward " : "jump backward ");

    int jump = Rede_printByteCode_parseJumpSize(iterator);
    printf("%d bytes\n", jump);

    return 0;
}

int Rede_printByteCode_jumpIfNot(RedeByteIterator* iterator) {
    printf("If not ");
    Rede_printByteCode_type(iterator);
    printf(RedeByteIterator_nextByte(iterator) == 0 ? " jump forward " : "jump backward ");
    
    int jump = Rede_printByteCode_parseJumpSize(iterator);
    printf("%d bytes\n", jump);

    return 0;
}

int Rede_printByteCode_stackPush(RedeByteIterator* iterator) {
    printf("Put on the stack ");
    Rede_printByteCode_type(iterator);
    printf("\n");

    return 0;
}

int Rede_printByteCode_stackClear(RedeByteIterator* iterator __attribute__((unused))) {
    printf("Clear the stack\n");
    
    return 0;
}

typedef struct Rede_printByteCode_Printer {
    int (*print)(RedeByteIterator* iterator);
} Rede_printByteCode_Printer;

Rede_printByteCode_Printer Rede_printByteCodePrinters[] = {
    [REDE_CODE_ASSIGN] = { Rede_printByteCode_assignment },
    [REDE_CODE_CALL] = { Rede_printByteCode_call },
    [REDE_CODE_JUMP] = { Rede_printByteCode_jump },
    [REDE_CODE_JUMP_IF] = { Rede_printByteCode_jumpIf },
    [REDE_CODE_JUMP_IF_NOT] = { Rede_printByteCode_jumpIfNot },
    [REDE_CODE_STACK_PUSH] = { Rede_printByteCode_stackPush },
    [REDE_CODE_STACK_CLEAR] = { Rede_printByteCode_stackClear },
};


void Rede_printBytecode(RedeByteCode* code) {
    RedeByteIterator iterator;

    RedeByteIterator_init(code, &iterator);


    unsigned char byte;
    while((byte = RedeByteIterator_nextByte(&iterator)) != REDE_CODE_END) {
        if(byte == REDE_CODE_NOP) {
            printf(" - NOP\n");
        } else if(byte < sizeof(Rede_printByteCodePrinters) / sizeof(Rede_printByteCode_Printer)) {
            Rede_printByteCode_Printer* printer = Rede_printByteCodePrinters + byte;
            printf(" - ");
            if(printer->print(&iterator) < 0) {
                return;
            }
        } else {
            printf("Unknown instruction %d\n", byte);
            return;
        }
    }
    printf(" - END\n");


    RedeByteIterator_destroy(&iterator);
}