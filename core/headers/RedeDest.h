#include "RedeCompiler.h"

#if !defined(REDE_DEST_H)
#define REDE_DEST_H

int RedeDest_init(RedeDest* dest);
void RedeDest_destroy(RedeDest* dest);
int RedeDest_writeByte(RedeDest* dest, unsigned char byte);
void RedeDest_moveCursorBack(RedeDest* dest, size_t n);
int RedeDest_writeByteAt(RedeDest* dest, size_t index, unsigned char byte);

#endif // REDE_DEST_H
