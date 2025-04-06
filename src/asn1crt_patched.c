/* Implementation for extended ASN1SCC runtime */
#include "asn1crt_patched.h"

void ExtendedBitStream_Init(ExtendedBitStream* bs, byte* buf, long count, MemPool* pool) {
    bs->buf = buf;
    bs->count = count;
    bs->currentByte = 0;
    bs->currentBit = 0;
    bs->memPool = pool;
}

void ExtendedBitStream_ToStandard(ExtendedBitStream* ebs, BitStream* bs) {
    bs->buf = ebs->buf;
    bs->count = ebs->count;
    bs->currentByte = ebs->currentByte;
    bs->currentBit = ebs->currentBit;
}
