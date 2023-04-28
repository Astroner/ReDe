#define REDE_IMPLEMENTATION
#include "../Rede.h"

#include "testing.h"

#include <assert.h>

void works() {
    Rede_createStringSource(c,
        "a = 2 "
        "b = 3 "
        "c = 'hi!' "
        "log(a b c) "
    );

    Rede_createCompilationMemory(cm, 3);

    Rede_createBufferDest(dest, 200);

    assert(Rede_compile(c, cm, dest) == 0);

    Rede_createByteCodeFromBuffer(bc, dest->data.buffer.buffer);

    Rede_createRuntimeMemory(rm, 4, 3, 100);

    assert(Rede_execute(bc, rm, Rede_std, NULL) == 0);
}

int main(void) {
    printf("\nGeneral tests:\n");

    TEST(works);

    printf("\n");
    return 0;
}