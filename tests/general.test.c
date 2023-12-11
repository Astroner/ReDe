#include "tests-new.h"

#include "tests.h"

#include "../Rede.h"

DESCRIBE(general) {
    IT("works") {
        Rede_createStringSource(c,
            "a = 2 "
            "b = 3 "
            "c = 'hi!' "
            "log(a b c) "
        );

        Rede_createCompilationMemory(cm, 3);

        Rede_createBufferDest(dest, 200);

        EXPECT(Rede_compile(c, cm, dest)) TO_BE(0)

        Rede_createByteCodeFromBuffer(bc, dest->data.buffer.buffer);

        Rede_createRuntimeMemory(rm, 4, 3, 100);

        EXPECT(Rede_execute(bc, rm, Rede_std, NULL)) TO_BE(0)
    }
}
