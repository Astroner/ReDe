#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RedeCompiler.h"
#include "RedeRuntime.h"
#include "RedeRuntimeUtils.h"
#include "RedeByteCodes.h"
#include "RedeStd.h"

int main(void) {
    // Rede_createStringSource(
    //     code,
    //     "log(length('hi!'))"
    // );

    Rede_createFileSource(code, "main.rede");

    Rede_createCompilationMemory(memory, 256);

    Rede_createFileDest(dest, "main.rd");

    int status = Rede_compile(code, memory, dest);

    if(status < 0) return 1;

    FILE* f = fopen("main.rd", "rb");

    int ch;
    while((ch = getc(f)) != EOF) {
        printf("%d ", ch);
    }

    printf("\n");
    fclose(f);

    return 0;
}