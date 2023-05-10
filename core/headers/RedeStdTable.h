#include "RedeStd.h"

#if !defined(REDE_STD_TABLE_H)
#define REDE_STD_TABLE_H

typedef struct RedeStdFunction {
    char* name;
    int(*call)(const RedeFunctionArgs* args, RedeVariable* result);
} RedeStdFunction;

extern size_t Rede_std_functions_size;
extern RedeStdFunction Rede_std_functions[];

#endif // REDE_STD_TABLE_H
