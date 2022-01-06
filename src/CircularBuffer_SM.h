#ifndef _SMARTMATRIX_CIRCULARBUFFER_H_
#define _SMARTMATRIX_CIRCULARBUFFER_H_

#include "MatrixCommon.h"

// TODO: Consider INLINE for several functions - many small, only used in one place, in frequently used code

/* Circular buffer object */
typedef struct {
    UINT16         size;   /* maximum number of elements           */
    UINT16         start;  /* index of oldest element              */
    UINT16         count;    /* new  */
} CircularBuffer_SM;

void cbInit(CircularBuffer_SM *cb, UINT16 size);

UINT16 cbIsFull(CircularBuffer_SM *cb);

UINT16 cbIsEmpty(CircularBuffer_SM *cb);

// returns index of next element to write
UINT16 cbGetNextWrite(CircularBuffer_SM *cb);

// mark next element as written
void cbWrite(CircularBuffer_SM *cb);

// returns index of next element to read
UINT16 cbGetNextRead(CircularBuffer_SM *cb);

// marks next element as read
void cbRead(CircularBuffer_SM *cb);


#endif // _SMARTMATRIX_CIRCULARBUFFER_H_
