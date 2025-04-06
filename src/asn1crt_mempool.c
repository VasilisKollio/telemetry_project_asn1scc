#include "asn1crt_mempool.h"

void MemPool_Init(MemPool* pool, byte* buffer, size_t size) {
    pool->buffer = buffer;
    pool->size = size;
    pool->used = 0;
}

void* MemPool_Alloc(MemPool* pool, size_t size) {
    size = (size + 3) & ~3; // 4-byte align
    if (pool->used + size > pool->size) return NULL;
    void* ptr = pool->buffer + pool->used;
    pool->used += size;
    return ptr;
}

void MemPool_Reset(MemPool* pool) {
    pool->used = 0;
}
