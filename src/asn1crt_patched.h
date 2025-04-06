/* ASN1SCC Runtime - Extended for partial decoding and streaming */
#ifndef ASN1SCC_RUNTIME_H
#define ASN1SCC_RUNTIME_H

/* Include original ASN1SCC runtime header */
#include "asn1crt.h"

/* Include our extensions */
#include "asn1crt_partial.h"
#include "asn1crt_stream.h"
#include "asn1crt_mempool.h"

/* Extended BitStream type with memory pool support */
typedef struct {
    byte* buf;           /* Buffer for PER encoding/decoding */
    long count;          /* Buffer size */
    long currentByte;    /* Current byte position */
    int currentBit;      /* Current bit position within byte */
    MemPool* memPool;    /* Optional memory pool */
} ExtendedBitStream;

/* Initialize extended bitstream */
void ExtendedBitStream_Init(ExtendedBitStream* bs, byte* buf, long count, MemPool* pool);

/* Function to get standard BitStream from extended one */
void ExtendedBitStream_ToStandard(ExtendedBitStream* ebs, BitStream* bs);

#endif /* ASN1SCC_RUNTIME_H */
