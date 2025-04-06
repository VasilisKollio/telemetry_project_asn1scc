/* asn1crt_stream.c - Streaming implementation for ASN1SCC */
#include "asn1crt_stream.h"
#include <string.h>

void StreamContext_Init(StreamContext* ctx) {
    memset(ctx, 0, sizeof(StreamContext));
    ctx->state = STREAM_INIT;
}

flag StreamContext_AddFragment(StreamContext* ctx, byte* data, size_t size) {
    if (ctx->fragmentCount >= MAX_STREAM_FRAGMENTS) {
        ctx->state = STREAM_ERROR;
        return FALSE;
    }
    
    ctx->fragments[ctx->fragmentCount].data = data;
    ctx->fragments[ctx->fragmentCount].size = size;
    ctx->fragments[ctx->fragmentCount].processed = 0;
    ctx->fragmentCount++;
    
    if (ctx->state == STREAM_INIT) {
        ctx->state = STREAM_PROCESSING;
    }
    
    return TRUE;
}

flag StreamContext_GetBitStream(StreamContext* ctx, BitStream* bs, int* bytesRead) {
    *bytesRead = 0;
    
    if (ctx->state != STREAM_PROCESSING) {
        return FALSE;
    }
    
    if (ctx->currentFragment < ctx->fragmentCount) {
        StreamFragment* fragment = &ctx->fragments[ctx->currentFragment];
        size_t remaining = fragment->size - fragment->processed;
        
        if (remaining > 0) {
            /* Attach remaining bytes to the bitstream */
            BitStream_AttachBuffer(bs, fragment->data + fragment->processed, remaining);
            *bytesRead = remaining;
            fragment->processed += remaining;
            return TRUE;
        } else {
            ctx->currentFragment++;
            return StreamContext_GetBitStream(ctx, bs, bytesRead);
        }
    }
    
    ctx->state = STREAM_COMPLETE;
    return FALSE;
}

flag StreamContext_IsComplete(StreamContext* ctx) {
    return ctx->state == STREAM_COMPLETE;
}

void StreamContext_Reset(StreamContext* ctx) {
    ctx->state = STREAM_INIT;
    ctx->fragmentCount = 0;
    ctx->currentFragment = 0;
}
