#include "tests-new.h"

#define MULTI_TEST
#include "tests.h"

#include "general.test.c"
#include "compiler.test.c"
#include "runtime.test.c"
#include "std.test.c"

RUN_TESTS(general, compiler, runtime, std)

#define REDE_IMPLEMENTATION
#include "../Rede.h"