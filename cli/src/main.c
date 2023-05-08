#include "main.h"

#include "options.h"

int main(int argc, char** argv) {
    Options options;
    Options_parse(argc, argv, &options);

    if(options.file.type != FileTypeNone) fileInput(&options);
    else if(options.help) printHelp();
    else realtime(&options);

    return 0;
}