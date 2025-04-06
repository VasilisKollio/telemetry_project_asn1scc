/* asn1crt_partial.h - Partial decoding support for ASN1SCC */
#ifndef ASN1CRT_PARTIAL_H
#define ASN1CRT_PARTIAL_H

#include "asn1crt.h"

/* Field selection structure to indicate which fields to decode */
typedef struct {
    int fieldIndex;       /* Index of field in SEQUENCE */
    const char* fieldName; /* Name for debugging */
    flag decode;          /* Whether to decode this field */
} FieldSelector;

/* Context for partial decoding operations */
typedef struct {
    FieldSelector* fields;  /* Array of field selectors */
    int fieldCount;         /* Number of fields in array */
    int currentField;       /* Current field being processed */
    int currentLevel;       /* Current nesting level */
} PartialContext;

/* Initialize a partial decoding context */
void PartialContext_Init(PartialContext* ctx, FieldSelector* fields, int fieldCount);

/* Check if field should be decoded */
flag ShouldDecodeField(PartialContext* ctx, int fieldIndex);

/* Move to next field in context */
void AdvanceField(PartialContext* ctx);

/* Increment nesting level */
void EnterLevel(PartialContext* ctx);

/* Decrement nesting level */
void ExitLevel(PartialContext* ctx);

#endif /* ASN1CRT_PARTIAL_H */
