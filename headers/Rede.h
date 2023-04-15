#if !defined(REDE_H)
#define REDE_H

#include <stdio.h>

typedef enum RedeSourceType {
    RedeSourceTypeFile,
    RedeSourceTypeString,
} RedeSourceType;

typedef struct RedeSource {
    RedeSourceType type;
    char* data;
} RedeSource;

int Rede_compile(RedeSource* src, char* buffer, size_t bufferLength);

typedef enum RedeVariableType {
    RedeVariableTypeNumber,
    RedeVariableTypeString,
} RedeVariableType;

typedef struct RedeVariable {
    RedeVariableType type;
    union {
        float number;
        struct {
            char* string;
            int length;
        } string;
    } data;
} RedeVariable;

typedef struct RedeFunctionArgs {
    RedeVariable* values;
    int length;
} RedeFunctionArgs;

#define REDE_TYPE_NUMBER 0
#define REDE_TYPE_STRING 1
#define REDE_TYPE_VAR 2
#define REDE_TYPE_STACK 3

#define REDE_CODE_ASSIGN 0
#define REDE_CODE_STACK_PUSH 1
#define REDE_CODE_CALL 2
#define REDE_CODE_END 255

int Rede_execute(
    unsigned char* program, 
    int (*)(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData),
    void* sharedData
);

void Rede_printVariable(RedeVariable*);
void Rede_printlnVariable(RedeVariable*);
void Rede_setNumber(RedeVariable* variable, float number);
void Rede_setString(RedeVariable* variable, char* string, size_t length);

#endif // REDE_H
