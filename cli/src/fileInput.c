#include "main.h"

#include "Rede.h"

void fileInput(Options* options) {
    if(options->file.type == FileTypeRD) {
        Rede_createByteCodeFromFile(src, options->file.path);

        Rede_createRuntimeMemory(runtime, 256, 256, 1024);

        Rede_execute(src, runtime, Rede_std, NULL);

        return;
    }

    Rede_createFileSource(src, options->file.path);
    Rede_createCompilationMemory(compilation, 256);

    RedeDest dest;
    if(options->compile) {
        dest.type = RedeDestTypeFile;
        if(options->outPath) dest.data.file.path = options->outPath;
        else dest.data.file.path = "out.rd";
    } else {
        unsigned char buffer[1024];
        memset(buffer, 0, 1024);
        dest.type = RedeDestTypeBuffer;
        dest.data.buffer.buffer = buffer;
        dest.data.buffer.length = 0;
        dest.data.buffer.maxLength = 1024;
    }

    int compilationStatus = Rede_compile(src, compilation, &dest);
    if(compilationStatus < 0) {
        printf("Failed to compile\n");
        return;
    }

    if(options->compile && !options->postCompilationExecution) return;

    RedeByteCode bytes;

    if(options->compile) {
        bytes.type = RedeByteCodeTypeFile;
        bytes.data.file.path = dest.data.file.path;
    } else {
        bytes.type = RedeByteCodeTypeBuffer;
        bytes.data.buffer.buffer = dest.data.buffer.buffer;
    }
    
    Rede_createRuntimeMemory(runtime, 256, 256, 1000);

    int executionStatus = Rede_execute(&bytes, runtime, Rede_std, NULL);

    if(executionStatus < 0) {
        printf("Failed to execute\n");
    }
}