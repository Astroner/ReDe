#include "RedeDest.h"
#include "logs.h"


int RedeDest_init(RedeDest* dest) {
    LOGS_SCOPE(RedeDest_init);
    switch(dest->type) {
        case RedeDestTypeFile: {
            dest->data.file.fp = fopen(dest->data.file.path, "wb");
            if(!dest->data.file.fp) {
                LOG_LN("Failed to open file '%s'", dest->data.file.path);
                return -1;
            }
            return 0;
        }

        case RedeDestTypeBuffer:
            return 0;

        default:
            LOG_LN("Unknown destination type");
            return -1;
    }
    return 0;
}

void RedeDest_destroy(RedeDest* dest) {
    LOGS_SCOPE(RedeDest_destroy);
    if(dest->type == RedeDestTypeFile) {
        fclose(dest->data.file.fp);
    }
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

        case RedeDestTypeFile:
            if(fputc(byte, dest->data.file.fp) != byte) {
                return -1;
            };
            return 0;

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
    
        case RedeDestTypeFile:
            fseek(dest->data.file.fp, -n - 1, SEEK_CUR);
            return;
            
        default:
            return;
    }
}