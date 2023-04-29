#include "options.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static FileType checkFile(char* path) {
    char ch;

    char* extensionStart = NULL;
    size_t index = 0;
    while((ch = path[index])) {
        if(ch == '.') {
            extensionStart = path + index + 1;
        }
        index++;
    }

    if(extensionStart == NULL) return FileTypeNone;
    if(strcmp(extensionStart, "rede") == 0) return FileTypeRede;
    if(strcmp(extensionStart, "rd") == 0) return FileTypeRD;

    return FileTypeNone;
}

static int parseCompileFlag(int argc, char** argv, int flagIndex, int flagNameLength, Options* result) {
    result->compile = 1;
    
    char* valueStart = argv[flagIndex] + flagNameLength;
    if(valueStart[0] == '=' && checkFile(valueStart + 1) == FileTypeRD) {
        result->outPath = valueStart + 1;
        return 0;
    } else if(flagIndex + 1 < argc && checkFile(argv[flagIndex + 1]) == FileTypeRD) {
        result->outPath = argv[flagIndex + 1];
        return 1;
    }

    return 0;
}

void Options_parse(int argc, char** argv, Options* result) {
    memset(result, 0, sizeof(Options));

    if(argc <= 1) return;

    for(int i = 1; i < argc; i++) {
        int compileFlagLength = 0;
        if(strncmp(argv[i], "--compile", 9) == 0) {
            compileFlagLength = 9;
        } else if(strncmp(argv[i], "-c", 2) == 0) {
            compileFlagLength = 2;
        }

        if(compileFlagLength > 0) {
            int status = parseCompileFlag(argc, argv, i, compileFlagLength, result);
            if(status == 1) {
                i++;
            }
        } else {
            FileType type = checkFile(argv[i]);
            if(type == FileTypeRede || type == FileTypeRD) {
                result->file.path = argv[i];
                result->file.type = type;
            } else {
                printf("Unknown option '%s'\n", argv[i]);
                exit(1);
            }
        }
    }
}