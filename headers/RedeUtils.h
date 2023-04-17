#if !defined(REDE_UTILS_H)
#define REDE_UTILS_H

#include "RedeRuntime.h"

void Rede_printVariable(RedeVariable*);
void Rede_printlnVariable(RedeVariable*);
void Rede_setNumber(RedeVariable* variable, float number);
void Rede_setString(RedeVariable* variable, char* string, size_t length);
void Rede_printMemory(RedeRuntimeMemory*);

#endif // REDE_UTILS_H
