#include "main.h"

#include "options.h"

int main(int argc, char** argv) {
    Options options;
    Options_parse(argc, argv, &options);

    if(options.file.type == FileTypeRede || options.file.type == FileTypeRD) fileInput(&options);
    else if(options.help) printHelp();
    else realtime();

    return 0;
}