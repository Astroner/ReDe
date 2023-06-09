#include "RedeRuntime.h"

#if !defined(REDE_RUNTIME_UTILS_H)
#define REDE_RUNTIME_UTILS_H

void Rede_printVariable(RedeVariable*);
void Rede_printlnVariable(RedeVariable*);
void Rede_setNumber(RedeVariable* variable, float number);
void Rede_setString(RedeVariable* variable, char* string, size_t length);
void Rede_setBoolean(RedeVariable* variable, int value);
void Rede_printMemory(RedeRuntimeMemory*);
void Rede_printBytecode(RedeByteCode* code);

#endif // REDE_RUNTIME_UTILS_H
