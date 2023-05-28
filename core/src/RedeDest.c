#include "RedeDest.h"
#include "logs.h"


int RedeDest_init(RedeDest* dest) {
    LOGS_SCOPE();

    dest->index = -1;

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
    LOGS_SCOPE();
    if(dest->type == RedeDestTypeFile) {
        fclose(dest->data.file.fp);
    }
}

int RedeDest_writeByte(RedeDest* dest, unsigned char byte) {

    dest->index++;
    switch(dest->type) {
        case RedeDestTypeBuffer:
            if(dest->index != dest->data.buffer.maxLength) {
                dest->data.buffer.buffer[dest->index] = byte;
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
    dest->index -= n;
    switch(dest->type) {    
        case RedeDestTypeFile:
            fseek(dest->data.file.fp, -n, SEEK_CUR);
            return;
            
        default:
            return;
    }
}

int RedeDest_writeByteAt(RedeDest* dest, size_t index, unsigned char byte) {
    switch(dest->type) {    
        case RedeDestTypeBuffer:
            if(index >= dest->data.buffer.maxLength) return -1;
            dest->data.buffer.buffer[index] = byte;
            return 0;

        case RedeDestTypeFile: 
            fseek(dest->data.file.fp, index, SEEK_SET);
            if(fputc(byte, dest->data.file.fp) != byte) {
                return -1;
            }
            fseek(dest->data.file.fp, dest->index + 1, SEEK_SET);
            return 0;
            
        default:
            return -1;
    }
}