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

int RedeCompilerHelpers_isToken(char* token, size_t tokenStart, size_t tokenLength, RedeSourceIterator* iterator) {

    size_t i = 0;
    while(1) {
        if(i == tokenLength) {
            return token[i] == '\0';
        }

        char tokenChar = token[i];
        char strChar = RedeSourceIterator_charAt(iterator, tokenStart + i);
        
        if(tokenChar == '\0' || tokenChar != strChar) return 0;
        
        i++;
    }
}

int RedeCompilerHelpers_nextTokenIs(char* token, RedeSourceIterator* iterator) {
    LOGS_SCOPE(nextTokenIs);

    LOG_LN("Checking for token '%s'", token);

    size_t tokenIndex = 0;
    char ch;
    while((ch = RedeSourceIterator_nextChar(iterator))) {
        LOG_LN("Char: %c(%d)", ch, ch);

        if(ch == ' ' || ch == '\n' || ch == '\r' || ch == 9/* TAB */) {
            if(tokenIndex > 0) {
                LOG_LN("Mismatch, reverting %zu token chars", tokenIndex + 1)
                RedeSourceIterator_moveCursorBack(iterator, tokenIndex + 1);
                return 0;
            }

            continue;
        } else if(ch == token[tokenIndex]) {
            tokenIndex++;
            if(token[tokenIndex] == '\0') return 1;
        } else {
            LOG_LN("Mismatch, reverting %zu token chars", tokenIndex + 1)
            RedeSourceIterator_moveCursorBack(iterator, tokenIndex + 1);
            
            return 0;
        }
    }
}

RedeExpressionWriteStatus RedeCompilerHelpers_writeBoolean(int value, RedeDest* dest) {
    LOGS_SCOPE(writeBoolean);

    CHECK(RedeDest_writeByte(dest, REDE_TYPE_BOOL), "Failed to write REDE_TYPE_BOOL");
    CHECK(RedeDest_writeByte(dest, value == 0 ? 0 : 1), "Failed to write boolean value");

    return RedeExpressionWriteStatusOk;
}