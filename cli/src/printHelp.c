#include "main.h"

#include <stdio.h>

typedef struct Flag {
    char* name;
    size_t nameLength;
    char** description;
} Flag;


#define MAX_NAME_LENGTH 15

char* helpDescription[] = {
    "Get CLI tool help",
    NULL
};

char* compileDescription[] = {
    "If .rede input file is provided, compiles it into .rd file.",
    "Add .rd filename after the flag to specify output name.",
    "Examples:",
    "rede --compile main.rede",
    "rede --compile main.rd main.rede",
    NULL
};

char* postCompilationExecutionDescription[] = {
    "Add this flag to enable post compilation execution.",
    "By default rede doesn't execute the code after the compilation,",
    "but with this flag rede will automaticaly execute generated .rd file",
    NULL
};

char* printBytecodeDescription[] = {
    "Print program bytecode",
    NULL
};

Flag flags[] = {
    {
        .name = "--help, -h",
        .nameLength = 11,
        helpDescription
    },
    {
        .name = "--compile, -c",
        .nameLength = 14,
        compileDescription
    },
    {
        .name = "--execute, -e",
        .nameLength = 14,
        postCompilationExecutionDescription
    },
    {
        .name = "--bytecode, -b",
        .nameLength = 15,
        printBytecodeDescription
    }
};

#define DESCRIPTION_SPACES 4

void printHelp() {
    printf("ReDe language CLI\n\nFlags:\n");

    for(size_t i = 0; i < sizeof(flags) / sizeof(Flag); i++) {
        size_t nameDiff = MAX_NAME_LENGTH - flags[i].nameLength;

        for(size_t j = 0; j < nameDiff; j++) {
            printf(" ");
        }

        printf("%s", flags[i].name);
        for(size_t j = 0; j < DESCRIPTION_SPACES; j++) {
            printf(" ");
        }

        printf("%s\n", flags[i].description[0]);
        char** cursor = flags[i].description + 1;
        char* line;
        while((line = *(cursor++))) {
            for(size_t j = 0; j < MAX_NAME_LENGTH + DESCRIPTION_SPACES - 1; j++) {
                printf(" ");
            }
            printf("%s\n", line);
        }
        printf("\n");
    }
}