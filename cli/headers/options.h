#if !defined(OPTIONS_H)
#define OPTIONS_H

typedef enum FileType {
    FileTypeNone,
    FileTypeRede,
    FileTypeRD
} FileType;

typedef struct Options {
    struct {
        FileType type;
        char* path;
    } file;
    int compile;
    char* outPath;
    int help;
} Options;

void Options_parse(int argc, char** argv, Options* result);

#endif // OPTIONS_H
