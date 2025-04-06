#include <stdio.h>
#include "asn1crt.h"
#include "asn1crt_mempool.h"
#include "satellite.h"

void hexdump(const char* desc, const void* addr, size_t len) {
    printf("%s (%zu bytes):\n", desc, len);
    const unsigned char* pc = (const unsigned char*)addr;
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", pc[i]);
        if ((i+1) % 8 == 0) printf("\n");
    }
    printf("\n");
}

void test_decoding() {
    byte buffer[4096];
    MemPool pool;
    MemPool_Init(&pool, buffer, sizeof(buffer));

    T_TelemetryFrame* frame = MemPool_Alloc(&pool, sizeof(T_TelemetryFrame));
    if (!frame) {
        printf("Allocation failed for %zu bytes\n", sizeof(T_TelemetryFrame));
        return;
    }

    /* Verified PER-encoded test data */
    byte test_data[] = {
        /* Header (9 bytes) */
        0x00, 0x00, 0x00, 0x01,  // seconds=1
        0x03, 0xE8,               // subseconds=1000
        0x00,                     // frameType=0
        0x00, 0x01,               // frameCount=1
        
        /* Payload (11 bytes) */
        0x80,                     // CHOICE tag (0x80 = [APPLICATION 0])
        0x13, 0x88,               // mainBus=5000mV
        0x0D, 0xAC,               // payload=3500mV
        0x09, 0xC4,               // comms=2500mV
        0x01,                     // temperature count=1
        0x01, 0x2C,               // temperature=300 (30.0°C)
        0x80                      // status=0x80
    };

    hexdump("Test Data", test_data, sizeof(test_data));

    BitStream bs;
    BitStream_Init(&bs, test_data, sizeof(test_data));
    
    int errCode;
    if (T_TelemetryFrame_Decode(frame, &bs, &errCode)) {
        printf("\n=== DECODE SUCCESS ===\n");
        printf("Bytes processed: %ld/%zu\n", bs.currentByte, sizeof(test_data));
        printf("Frame count: %lu\n", frame->header.frameCount);
        printf("Payload type: %d\n", frame->payload.kind);
        
        if (frame->payload.kind == 0) {
            printf("Housekeeping data:\n");
            printf("- Main bus: %lu mV\n", frame->payload.u.housekeeping.voltages.mainBus);
            printf("- Temperature: %ld °C\n", frame->payload.u.housekeeping.temperature.arr[0]/10);
        } else {
            printf("UNEXPECTED payload type! Check CHOICE tags.\n");
            printf("First payload byte: 0x%02x\n", test_data[9]);
        }
    } else {
        printf("\nDECODE FAILED (Error: %d)\n", errCode);
        printf("Bytes processed: %ld/%zu\n", bs.currentByte, sizeof(test_data));
    }
}

int main() {
    printf("===== Telemetry Decoder Test =====\n");
    test_decoding();
    return 0;
}
