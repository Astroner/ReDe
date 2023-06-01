#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeSourceIterator.h"
#include "RedeCompiler.h"



RedeWriteStatus RedeCompilerHelpers_parseComment(RedeSourceIterator* src) {
    LOGS_SCOPE(parseComment);

    char ch;
    while((ch = RedeSourceIterator_nextChar(src))) {
        LOG_LN("Char: '%c'(%d)", ch, ch);

        int end = 0;

        switch(ch) {
            case '\r':
            case '\n':
                LOG_LN("End of the comment");
                end = 1;
                break;
        }

        if(end) break;
    }

    return RedeWriteStatusOk;
}