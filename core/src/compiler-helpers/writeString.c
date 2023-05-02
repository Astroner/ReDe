#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




int RedeCompilerHelpers_writeString(
    int singleQuoted, 
    RedeSourceIterator* iterator,
    RedeDest* dest,
    RedeCompilationContext* ctx
) {
    LOGS_SCOPE(writeString);

    LOG_LN(singleQuoted ? "Single quoted" : "Double quoted");


    size_t stringStart = iterator->index + 1;
    size_t pureStringLength = 0;
    size_t iteratedChars = 0;

    int endedWithQuotes = 0;
    int isBackSlashed = 0;

    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(endedWithQuotes) {
            LOG_LN("Did an extra iteration after the string end coz of function call");
            break;
        }

        if(isBackSlashed) {
            LOG_LN("Back-slashed");
            isBackSlashed = 0;
            pureStringLength++;
        } else if((singleQuoted && ch == '\'') || (!singleQuoted && ch == '"')) {
            LOG_LN("String end");

            endedWithQuotes = 1;


            if(ctx->functionCallDepth == 0) {
                break;
            } else {
                LOG_LN("But need to check the next char because it is function call");
                continue;
            }
        } else if (ch == '\\') {
            LOG_LN("Back-slash mark");
            
            isBackSlashed = 1;
        } else {
            pureStringLength++;
        }
        
        iteratedChars++;
    }

    if(!endedWithQuotes) {
        LOG_LN("Unexpected end of the string");

        return -2;
    }

    LOG_LN("Chars iterated: %zu", iteratedChars);
    LOG_LN("String length: %zu", pureStringLength);


    if(pureStringLength > 255) {
        LOG_LN("String length overflow (%zu > 255)", pureStringLength);

        return -3;
    }

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_STRING), 0, "Failed to write REDE_TYPE_STRING");

    CHECK(RedeDest_writeByte(dest, (unsigned char)pureStringLength), 0, "Failed to write string length");

    LOG_LN("Writing to the buffer");
    for(size_t i = stringStart; i < stringStart + iteratedChars; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(ch != '\\') {
            CHECK(RedeDest_writeByte(dest, (unsigned char)ch), 0, "Failed to write char");
        } 
        LOGS_ONLY(
            else {
                LOG_LN("Skiped as back-slash")
            }
        )
    }

    return 0;
}