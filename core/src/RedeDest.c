#include "RedeDest.h"
#include "logs.h"


int RedeDest_init(RedeDest* dest) {
    LOGS_SCOPE(RedeDest_init);
    if(dest->type != RedeDestTypeBuffer) {
        LOG_LN("Unknown destination type");
        return -1;
    }
    return 0;
}

int RedeDest_destroy(RedeDest* dest) {
    LOGS_SCOPE(RedeDest_destroy);
    if(dest->type != RedeDestTypeBuffer) {
        LOG_LN("Unknown destination type");
        return -1;
    }
    return 0;
}

int RedeDest_writeByte(RedeDest* dest, unsigned char byte) {

    switch(dest->type) {
        case RedeDestTypeBuffer:
            if(dest->data.buffer.length != dest->data.buffer.maxLength) {
                dest->data.buffer.buffer[dest->data.buffer.length] = byte;
                dest->data.buffer.length++;
                return 0;
            } else {
                return -1;
            }
        default:
            return -1;
    }
}

void RedeDest_moveCursorBack(RedeDest* dest, size_t n) {

    switch(dest->type) {
        case RedeDestTypeBuffer:
            if(dest->data.buffer.length >= n) {
                dest->data.buffer.length -= n;
            } else {
                dest->data.buffer.length = 0;
            }
            return;
        default:
            return;
    }
}