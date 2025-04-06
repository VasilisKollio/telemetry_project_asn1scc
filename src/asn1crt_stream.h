/* asn1crt_stream.h - Streaming support for ASN1SCC */
#ifndef ASN1CRT_STREAM_H
#define ASN1CRT_STREAM_H

#include "asn1crt.h"

void BitStream_AttachBuffer(BitStream* bs, byte* buf, int size);

/* Maximum number of fragments in a stream */
#define MAX_STREAM_FRAGMENTS 16

/* Stream processing state */
typedef enum {
    STREAM_INIT,
    STREAM_PROCESSING,
    STREAM_COMPLETE,
    STREAM_ERROR
} StreamState;

/* Stream fragment */
typedef struct {
    byte* data;       /* Fragment data */
    size_t size;      /* Size in bytes */
    size_t processed; /* Bytes already processed */
} StreamFragment;

/* Stream processing context */
typedef struct {
    StreamState state;
    StreamFragment fragments[MAX_STREAM_FRAGMENTS];
    int fragmentCount;
    int currentFragment;
    BitStream internalBitStream;
} StreamContext;

/* Initialize stream context */
void StreamContext_Init(StreamContext* ctx);

/* Add a fragment to the stream */
flag StreamContext_AddFragment(StreamContext* ctx, byte* data, size_t size);

/* Get bitstream for decoder to read from */
flag StreamContext_GetBitStream(StreamContext* ctx, BitStream* bs, int* bytesRead);

/* Check if stream is complete */
flag StreamContext_IsComplete(StreamContext* ctx);

/* Reset stream for reuse */
void StreamContext_Reset(StreamContext* ctx);

#endif /* ASN1CRT_STREAM_H */
