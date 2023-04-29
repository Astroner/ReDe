#include "main.h"

#include "Rede.h"
#include "options.h"


#define MAX_LINE_WIDTH 100

typedef struct RealtimeData {
    RedeRuntimeMemory* memory;
    int quit;
} RealtimeData;

int fCall(const char *name, size_t nameLength, const RedeFunctionArgs *args, RedeVariable *result, void *sharedData) {
    int status = Rede_std(name, nameLength, args, result, NULL);
    if(status != -2) return status;

    RealtimeData* data = sharedData;

    if(strcmp(name, "memory") == 0) {
        Rede_printMemory(data->memory);
    } else if(strcmp(name, "quit") == 0 || strcmp(name, "q") == 0) {
        data->quit = 1;
    } else {
        return -2;
    }

    return 0;
}

void realtime() {
    printf("Guten Tag!\n");
    printf("ReDe CLI v0.1\n");

    char buffer[MAX_LINE_WIDTH];
    memset(buffer, 0, MAX_LINE_WIDTH);

    Rede_createStringSource(code, buffer);
    Rede_createCompilationMemory(compilation, 256);
    Rede_createBufferDest(dest, 200);

    Rede_createRuntimeMemory(runtime, 100, 256, 1024);
    Rede_createByteCodeFromBuffer(bytes, dest->data.buffer.buffer);

    RealtimeData data = {
        .quit = 0,
        .memory = runtime,
    };
    
    while(!data.quit) {
        printf("> ");

        size_t inputLength = 0;
        size_t bracketsDepth = 0;
        size_t isString = 0;
        int doubleQuotes = 1;
        while(1) {
            char ch = getc(stdin);
            buffer[inputLength] = ch;
            inputLength++;
            
            if(ch == '(') {
                bracketsDepth++;
            } else if(ch == ')') {
                if(bracketsDepth > 0) bracketsDepth--;
            } else if(ch == '"' || ch == '\'') {
                if(isString && ((ch == '"') == doubleQuotes)) {
                    isString = 0;
                } else if(!isString) {
                    isString = 1;
                    doubleQuotes = ch == '"';
                }
            } else if(ch == '\n') {
                if(bracketsDepth == 0 && !isString) break;
                else {
                    printf(".");
                    for(size_t i = 0; i < bracketsDepth; i++) {
                        printf("..");
                    }
                    printf(" ");
                }
            }

            if(inputLength == MAX_LINE_WIDTH - 1) break;
        }
        int status = Rede_compile(code, compilation, dest);

        if(status < 0) {
            printf("Failed to compile\n");
            goto loop_end;
        }

        status = Rede_execute(bytes, runtime, fCall, &data);

        if(status < 0) {
            printf("Failed to execute\n");
        }
    
loop_end:
        memset(buffer, 0, MAX_LINE_WIDTH);
        dest->data.buffer.length = 0;
    }
}