#include "RedeRuntime.h"
#include "RedeRuntimeUtils.h"

#if !defined(REDE_STD_H)
#define REDE_STD_H

int Rede_std(const char* name, size_t nameLength, const RedeFunctionArgs* args, RedeVariable* result, void* sharedData);

#endif // REDE_STD_H
