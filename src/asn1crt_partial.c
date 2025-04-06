/* asn1crt_partial.c - Partial decoding implementation */
#include "asn1crt_partial.h"
#include <string.h>

void PartialContext_Init(PartialContext* ctx, FieldSelector* fields, int fieldCount) {
    ctx->fields = fields;
    ctx->fieldCount = fieldCount;
    ctx->currentField = 0;
    ctx->currentLevel = 0;
}

flag ShouldDecodeField(PartialContext* ctx, int fieldIndex) {
    /* If no context provided, decode everything */
    if (ctx == NULL || ctx->fields == NULL) {
        return TRUE;
    }
    
    /* Check if field is in our selector list */
    for (int i = 0; i < ctx->fieldCount; i++) {
        if (ctx->fields[i].fieldIndex == fieldIndex) {
            return ctx->fields[i].decode;
        }
    }
    
    /* Default to skipping fields not explicitly listed */
    return FALSE;
}

void AdvanceField(PartialContext* ctx) {
    if (ctx != NULL) {
        ctx->currentField++;
    }
}

void EnterLevel(PartialContext* ctx) {
    if (ctx != NULL) {
        ctx->currentLevel++;
    }
}

void ExitLevel(PartialContext* ctx) {
    if (ctx != NULL && ctx->currentLevel > 0) {
        ctx->currentLevel--;
    }
}
