/* CircularBuffer_SM.cpp
modified from example code from Wikipedia:
http://en.wikipedia.org/wiki/Circular_queue#Use_a_Fill_Count

modified to only contain indexes and not any data elements, and allow for peeking at the next read/write index
*/

#include "CircularBuffer_SM.h"


void cbInit(CircularBuffer_SM *cb, UINT16 size) {
    cb->size  = size;
    cb->start = 0;
    cb->count = 0;
}

/* below from fill count mods */
UINT16 cbIsFull(CircularBuffer_SM *cb) {
    return cb->count == cb->size;
}

UINT16 cbIsEmpty(CircularBuffer_SM *cb) {
    return cb->count == 0;
}

// returns index of next free element
UINT16 cbGetNextWrite(CircularBuffer_SM *cb) {
    return (cb->start + cb->count) % cb->size;
}

void cbRead(CircularBuffer_SM *cb) {
    cb->start = (cb->start + 1) % cb->size;
    -- cb->count;
}

void cbWrite(CircularBuffer_SM *cb) {
    if (cb->count == cb->size)
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
    else
        ++ cb->count;
}

UINT16 cbGetNextRead(CircularBuffer_SM *cb) {
    return cb->start;
}
