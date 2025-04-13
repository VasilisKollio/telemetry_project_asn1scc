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
        0x00,                     // CHOICE tag (0 for housekeeping)
        0x13, 0x88,               // mainBus=5000mV
        0x0D, 0xAC,               // payload=3500mV
        0x09, 0xC4,               // comms=2500mV
        0x01,                     // temperature count=1
        0x01, 0x2C,               // temperature=300 (30.0°C)
        0x80                      // status=0x80
    };

    // Print raw test data
    printf("Raw test data as seen by decoder:\n");
    for (int i = 0; i < sizeof(test_data); i++) {
        printf("%02x ", test_data[i]);
        if ((i+1) % 8 == 0) printf("\n");
    }
    printf("\n");

    hexdump("Test Data", test_data, sizeof(test_data));
    
    BitStream bs;
    BitStream_Init(&bs, test_data, sizeof(test_data));
    bs.currentByte = 0;
    bs.currentBit = 0;
    
    int errCode;
    if (T_TelemetryFrame_Decode(frame, &bs, &errCode)) {
        printf("\n=== DECODE SUCCESS ===\n");
        printf("Bytes processed: %ld/%zu\n", bs.currentByte, sizeof(test_data));
        printf("Frame count: %lu\n", frame->header.frameCount);
        printf("Payload type: %d\n", frame->payload.kind);
        
        // Debug outputs
        printf("CHOICE tag value in test data: 0x%02x\n", test_data[9]);
        printf("ASN.1 CHOICE index decoded: %d\n", frame->payload.kind);
        printf("Expected housekeeping_PRESENT value: %d\n", housekeeping_PRESENT);
        
        printf("After decoding:\n");
        printf("- Header timeStamp seconds: %lu\n", frame->header.timestamp.seconds);
        printf("- Header timeStamp subseconds: %lu\n", frame->header.timestamp.subseconds);
        printf("- Header frameType: %lu\n", frame->header.frameType);
        printf("- Header frameCount: %lu\n", frame->header.frameCount);
        printf("- Payload kind: %d\n", frame->payload.kind);
        
        if (frame->payload.kind == housekeeping_PRESENT) {
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

void test_with_generated_data() {
    // 1. Create a buffer for encoding
    byte encBuffer[4096];
    BitStream encBitStream;
    BitStream_Init(&encBitStream, encBuffer, sizeof(encBuffer));

    // 2. Create a TelemetryFrame with known values
    T_TelemetryFrame testFrame;
    T_TelemetryFrame_Initialize(&testFrame);

    // Set header values
    testFrame.header.timestamp.seconds = 1;
    testFrame.header.timestamp.subseconds = 1000;
    testFrame.header.frameType = 0;
    testFrame.header.frameCount = 1;

    // Set payload to housekeeping
    testFrame.payload.kind = housekeeping_PRESENT;
    testFrame.payload.u.housekeeping.voltages.mainBus = 5000;
    testFrame.payload.u.housekeeping.voltages.payload = 3500;
    testFrame.payload.u.housekeeping.voltages.comms = 2500;
    testFrame.payload.u.housekeeping.temperature.nCount = 1;
    testFrame.payload.u.housekeeping.temperature.arr[0] = 30; // 3.0°C (within -100 to 100 range)

    // Initialize status properly
    memset(testFrame.payload.u.housekeeping.status.arr, 0, sizeof(testFrame.payload.u.housekeeping.status.arr));
    testFrame.payload.u.housekeeping.status.arr[0] = 0x80; // Set the first bit only

    // Debugging for status field
    printf("Status field size: %lu bytes\n", sizeof(testFrame.payload.u.housekeeping.status.arr));
    printf("Status field value: 0x%02x\n", testFrame.payload.u.housekeeping.status.arr[0]);

    // 3. Encode the frame
    int errCode;
    if (T_TelemetryFrame_Encode(&testFrame, &encBitStream, &errCode, TRUE)) {
        printf("Encoding successful! Generated %ld bytes\n", encBitStream.currentByte);
        
        // Print the generated bytes
        printf("Generated PER-encoded data:\n");
        for (int i = 0; i < encBitStream.currentByte; i++) {
            printf("%02x ", encBuffer[i]);
            if ((i+1) % 8 == 0) printf("\n");
        }
        printf("\n");

        // 4. Now try to decode it
        BitStream decBitStream;
        BitStream_Init(&decBitStream, encBuffer, encBitStream.currentByte);
        
        T_TelemetryFrame decodedFrame;
        if (T_TelemetryFrame_Decode(&decodedFrame, &decBitStream, &errCode)) {
            printf("Decoding successful!\n");
            printf("Decoded values:\n");
            printf("- Seconds: %lu\n", decodedFrame.header.timestamp.seconds);
            printf("- Subseconds: %lu\n", decodedFrame.header.timestamp.subseconds);
            printf("- Frame type: %lu\n", decodedFrame.header.frameType);
            printf("- Frame count: %lu\n", decodedFrame.header.frameCount);
            printf("- Payload kind: %d\n", decodedFrame.payload.kind);
            
            if (decodedFrame.payload.kind == housekeeping_PRESENT) {
                printf("- Main bus: %lu mV\n", decodedFrame.payload.u.housekeeping.voltages.mainBus);
                printf("- Payload voltage: %lu mV\n", decodedFrame.payload.u.housekeeping.voltages.payload);
                printf("- Comms voltage: %lu mV\n", decodedFrame.payload.u.housekeeping.voltages.comms);
                printf("- Temperature: %d °C\n", decodedFrame.payload.u.housekeeping.temperature.arr[0]);
                printf("- Status: 0x%02x\n", decodedFrame.payload.u.housekeeping.status.arr[0]);
            }
        } else {
            printf("Decoding failed with error: %d\n", errCode);
        }

        // 5. Replace the test data in test_decoding() with this properly encoded data
        printf("\nTo fix your test_decoding() function, replace your test_data array with:\n");
        printf("byte test_data[] = {\n    ");
        for (int i = 0; i < encBitStream.currentByte; i++) {
            printf("0x%02x, ", encBuffer[i]);
            if ((i+1) % 8 == 0 && i < encBitStream.currentByte-1) printf("\n    ");
        }
        printf("\n};\n");
    } else {
        printf("Encoding failed with error: %d\n", errCode);
    }
}

int main() {
    printf("===== Telemetry Decoder Test =====\n");
    test_decoding();
    
    printf("\n===== Generated Test Vector Test =====\n");
    test_with_generated_data();
    
    return 0;
}
