#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




RedeExpressionWriteStatus RedeCompilerHelpers_writeString(
    int singleQuoted, 
    RedeSourceIterator* iterator,
    RedeDest* dest
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

        if(isBackSlashed) {
            LOG_LN("Back-slashed");
            isBackSlashed = 0;
            pureStringLength++;
        } else if((singleQuoted && ch == '\'') || (!singleQuoted && ch == '"')) {
            LOG_LN("String end");

            endedWithQuotes = 1;

            break;
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

        return RedeExpressionWriteStatusError;
    }

    LOG_LN("Chars iterated: %zu", iteratedChars);
    LOG_LN("String length: %zu", pureStringLength);


    if(pureStringLength > 255) {
        LOG_LN("String length overflow (%zu > 255)", pureStringLength);

        return RedeExpressionWriteStatusError;
    }

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_STRING), "Failed to write REDE_TYPE_STRING");

    CHECK(RedeDest_writeByte(dest, (unsigned char)pureStringLength), "Failed to write string length");

    LOG_LN("Writing to the buffer");
    for(size_t i = stringStart; i < stringStart + iteratedChars; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("CHAR: '%c'(%d)", ch, ch);

        if(ch != '\\') {
            CHECK(RedeDest_writeByte(dest, (unsigned char)ch), "Failed to write char");
        } 
        LOGS_ONLY(
            else {
                LOG_LN("Skiped as back-slash")
            }
        )
    }

    return 0;
}