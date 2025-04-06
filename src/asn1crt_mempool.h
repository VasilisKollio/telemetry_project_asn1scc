/* asn1crt_mempool.h - Memory pool for ASN1SCC */
#ifndef ASN1CRT_MEMPOOL_H
#define ASN1CRT_MEMPOOL_H

#include "asn1crt.h"  // Must include base runtime first

/* Simple memory pool structure */
typedef struct {
    byte* buffer;     /* Pool buffer */
    size_t size;      /* Total size */
    size_t used;      /* Bytes allocated */
} MemPool;

/* Initialization */
void MemPool_Init(MemPool* pool, byte* buffer, size_t size);

/* Allocation (returns NULL if out of memory) */
void* MemPool_Alloc(MemPool* pool, size_t size);

/* Reset pool (does NOT free memory) */
void MemPool_Reset(MemPool* pool);

#endif /* ASN1CRT_MEMPOOL_H */
