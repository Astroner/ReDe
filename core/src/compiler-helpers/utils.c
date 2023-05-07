#include "RedeCompilerHelpers.h"
#include "logs.h"
#include "RedeByteCodes.h"




unsigned long RedeCompilerHelpers_hash(
    RedeSourceIterator* iterator, 
    size_t identifierStart, 
    size_t identifierLength
) {
    LOGS_SCOPE(hash);

    unsigned long hash = 5381;
    for(size_t i = identifierStart; i < identifierStart + identifierLength; i++) {
        char ch = RedeSourceIterator_charAt(iterator, i);
        LOG_LN("%zu) '%c'(%d)", i, ch, ch);
        hash = ((hash << 5) + hash) + ch; /* hash * 33 + c */
    }

    return hash;
}

int RedeCompilerHelpers_isToken(char* token, size_t identifierStart, size_t identifierLength, RedeSourceIterator* iterator) {

    size_t i = 0;
    while(1) {
        if(i == identifierLength) {
            return token[i] == '\0';
        }

        char tokenChar = token[i];
        char strChar = RedeSourceIterator_charAt(iterator, identifierStart + i);
        
        if(tokenChar == '\0' || tokenChar != strChar) return 0;
        
        i++;
    }
}

int RedeCompilerHelpers_writeBoolean(int value, RedeDest* dest) {
    LOGS_SCOPE(writeBoolean);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_BOOL), 0, "Failed to write REDE_TYPE_BOOL");
    CHECK(RedeDest_writeByte(dest, value == 0 ? 0 : 1), 0, "Failed to write boolean value");

    return 0;
}