#include <stdio.h>
#include <string.h>
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
    if (len % 8 != 0) printf("\n");
    printf("\n");
}

void test_decoding() {
    printf("=== Testing with Hardcoded Test Data ===\n");
    
    unsigned char buffer[4096];
    MemPool pool;
    MemPool_Init(&pool, buffer, sizeof(buffer));

    T_TelemetryFrame* frame = (T_TelemetryFrame*)MemPool_Alloc(&pool, sizeof(T_TelemetryFrame));
    if (!frame) {
        printf("Allocation failed for %zu bytes\n", sizeof(T_TelemetryFrame));
        return;
    }

    // Initialize the allocated frame
    T_TelemetryFrame_Initialize(frame);

    /* Your original test data */
    unsigned char test_data[] = {
        0x00, 0x00, 0x00, 0x01, 0xfa, 0x00, 0x00, 0x00,
        0x49, 0xc4, 0x36, 0xb1, 0x38, 0x82, 0x0a
    };

    printf("Raw test data as seen by decoder:\n");
    for (size_t i = 0; i < sizeof(test_data); i++) {
        printf("%02x ", test_data[i]);
        if ((i+1) % 8 == 0) printf("\n");
    }
    printf("\n");

    hexdump("Test Data", test_data, sizeof(test_data));

    // CRITICAL FIX: Use BitStream_AttachBuffer for decoding
    BitStream bs;
    BitStream_AttachBuffer(&bs, test_data, sizeof(test_data));

    int errCode;
    if (T_TelemetryFrame_Decode(frame, &bs, &errCode)) {
        printf("\n=== DECODE SUCCESS ===\n");
        printf("Bytes processed: %ld/%zu\n", bs.currentByte, sizeof(test_data));
        printf("Frame count: %lu\n", frame->header.frameCount);
        printf("Payload type: %d\n", frame->payload.kind);

        printf("CHOICE tag value in test data: 0x%02x\n", test_data[9]);
        printf("ASN.1 CHOICE index decoded: %d\n", frame->payload.kind);
        printf("Expected housekeeping_PRESENT value: %d\n", housekeeping_PRESENT);
        printf("After decoding:\n");

        printf("- Header timeStamp seconds: %lu\n", frame->header.timestamp.seconds);
        printf("- Header timeStamp subseconds: %lu\n", frame->header.timestamp.subseconds);
        printf("- Header frameType: %lu\n", frame->header.frameType);
        printf("- Header frameCount: %lu\n", frame->header.frameCount);
        printf("- Payload kind: %u\n", frame->payload.kind);

        if (frame->payload.kind == housekeeping_PRESENT) {
            printf("Housekeeping data:\n");
            printf("- Main bus: %lu mV\n", frame->payload.u.housekeeping.voltages.mainBus);
            if (frame->payload.u.housekeeping.temperature.nCount > 0) {
                printf("- Temperature: %ld C\n", frame->payload.u.housekeeping.temperature.arr[0]);
            }
        } else {
            printf("UNEXPECTED payload type! Check CHOICE tags.\n");
            printf("Decoded payload kind: %d, expected: %d\n", frame->payload.kind, housekeeping_PRESENT);
        }
    } else {
        printf("\nDECODE FAILED (Error: %d)\n", errCode);
        printf("Bytes processed: %ld/%zu\n", bs.currentByte, sizeof(test_data));
        
        // Additional debug info
        printf("Stream position: bit %d, byte %ld\n", bs.currentBit, bs.currentByte);
        printf("This suggests the hardcoded data may not match current ASN.1 schema\n");
    }
}

void test_with_generated_data() {
    printf("=== Testing with Freshly Generated Data ===\n");
    
    // Create buffers for encoding/decoding
    unsigned char encBuffer[4096];
    BitStream encBitStream;
    BitStream_Init(&encBitStream, encBuffer, sizeof(encBuffer));

    // Create a proper frame with full initialization
    T_TelemetryFrame testFrame;
    T_TelemetryFrame_Initialize(&testFrame);

    // Set header values within valid constraints
    testFrame.header.timestamp.seconds = 1000000;        // Valid range: 0..4294967295
    testFrame.header.timestamp.subseconds = 500;         // Valid range: 0..1000
    testFrame.header.frameType = 1;                       // Valid range: 0..255
    testFrame.header.frameCount = 42;                     // Valid range: 0..65535

    // Set payload to housekeeping
    testFrame.payload.kind = housekeeping_PRESENT;
    
    // Initialize housekeeping data properly
    testFrame.payload.u.housekeeping.voltages.mainBus = 3300;    // 3.3V in mV
    testFrame.payload.u.housekeeping.voltages.payload = 5000;    // 5.0V in mV
    testFrame.payload.u.housekeeping.voltages.comms = 1800;      // 1.8V in mV
    
    // Set temperature array
    testFrame.payload.u.housekeeping.temperature.nCount = 2;
    testFrame.payload.u.housekeeping.temperature.arr[0] = 25;    // 25°C
    testFrame.payload.u.housekeeping.temperature.arr[1] = 30;    // 30°C
    
    // Initialize status bit string properly
    memset(&testFrame.payload.u.housekeeping.status, 0, sizeof(testFrame.payload.u.housekeeping.status));

    // Encode the frame
    int errCode;
    if (T_TelemetryFrame_Encode(&testFrame, &encBitStream, &errCode, TRUE)) {
        int encoded_length = BitStream_GetLength(&encBitStream);
        printf("Encoding successful! Generated %d bytes\n", encoded_length);

        // Print the generated bytes
        printf("Generated PER-encoded data:\n");
        for (int i = 0; i < encoded_length; i++) {
            printf("%02x ", encBuffer[i]);
            if ((i+1) % 8 == 0) printf("\n");
        }
        if (encoded_length % 8 != 0) printf("\n");

        // Try decoding the freshly generated data
        BitStream decBitStream;
        BitStream_AttachBuffer(&decBitStream, encBuffer, encoded_length);

        T_TelemetryFrame decodedFrame;
        T_TelemetryFrame_Initialize(&decodedFrame);
        
        if (T_TelemetryFrame_Decode(&decodedFrame, &decBitStream, &errCode)) {
            printf("Decoding successful!\n");
            
            // Verify the round-trip data integrity
            printf("=== Data Integrity Check ===\n");
            printf("Original -> Decoded\n");
            printf("Timestamp: %lu.%lu -> %lu.%lu\n",
                   testFrame.header.timestamp.seconds, testFrame.header.timestamp.subseconds,
                   decodedFrame.header.timestamp.seconds, decodedFrame.header.timestamp.subseconds);
            printf("Frame count: %lu -> %lu\n",
                   testFrame.header.frameCount, decodedFrame.header.frameCount);
            printf("Payload kind: %d -> %d\n",
                   testFrame.payload.kind, decodedFrame.payload.kind);
            
            if (decodedFrame.payload.kind == housekeeping_PRESENT) {
                printf("Voltage (mainBus): %lu -> %lu mV\n",
                       testFrame.payload.u.housekeeping.voltages.mainBus,
                       decodedFrame.payload.u.housekeeping.voltages.mainBus);
            }
            
            // Check if data matches
            int data_matches = (
                testFrame.header.timestamp.seconds == decodedFrame.header.timestamp.seconds &&
                testFrame.header.timestamp.subseconds == decodedFrame.header.timestamp.subseconds &&
                testFrame.header.frameCount == decodedFrame.header.frameCount &&
                testFrame.payload.kind == decodedFrame.payload.kind
            );
            
            printf("Data integrity: %s\n", data_matches ? "PASSED" : "FAILED");
            
        } else {
            printf("Decoding failed with error: %d\n", errCode);
            printf("This is unexpected since we just encoded this data successfully!\n");
        }
    } else {
        printf("Encoding failed with error: %d\n", errCode);
        
        // Provide debugging info for encoding failure
        switch(errCode) {
            case 147: // ERR_UPER_ENCODE_TELEMETRYFRAME
                printf("TelemetryFrame encoding error - check all fields are properly initialized\n");
                break;
            case 137: // ERR_UPER_ENCODE_TELEMETRYFRAME_HEADER
                printf("Header encoding error - check timestamp and frame fields\n");
                break;
            case 142: // ERR_UPER_ENCODE_TELEMETRYFRAME_PAYLOAD
                printf("Payload encoding error - check housekeeping data initialization\n");
                break;
            default:
                printf("Unknown encoding error - check constraint violations\n");
                break;
        }
    }
}

void test_minimal() {
    printf("=== Minimal Functionality Test ===\n");
    
    // Create a minimal TelemetryFrame with just enough to test
    T_TelemetryFrame frame;
    T_TelemetryFrame_Initialize(&frame);

    printf("Status field properly initialized by T_TelemetryFrame_Initialize()\n");

    // Set it to housekeeping with minimal valid data
    frame.payload.kind = housekeeping_PRESENT;
    
    // Set minimal valid header values
    frame.header.timestamp.seconds = 1;
    frame.header.timestamp.subseconds = 0;
    frame.header.frameType = 0;
    frame.header.frameCount = 1;

    // Encode
    unsigned char buffer[1024];
    BitStream bs;
    BitStream_Init(&bs, buffer, sizeof(buffer));

    int errCode;
    if (T_TelemetryFrame_Encode(&frame, &bs, &errCode, TRUE)) {
        int encoded_length = BitStream_GetLength(&bs);
        printf("Minimal encode successful: %d bytes\n", encoded_length);

        // Decode the same data
        BitStream decodeBs;
        BitStream_AttachBuffer(&decodeBs, buffer, encoded_length);

        T_TelemetryFrame decodedFrame;
        T_TelemetryFrame_Initialize(&decodedFrame);
        
        if (T_TelemetryFrame_Decode(&decodedFrame, &decodeBs, &errCode)) {
            printf("Minimal decode successful\n");
            printf("Round-trip test: PASSED\n");
        } else {
            printf("Minimal decode failed: error %d\n", errCode);
            printf("Round-trip test: FAILED\n");
        }
    } else {
        printf("Minimal encode failed: error %d\n", errCode);
        printf("Check if all required fields are properly initialized\n");
    }
}

int main() {
    printf("===== ASN.1 Telemetry Decoder Comprehensive Test =====\n");
    printf("Frame size: %zu bytes\n\n", sizeof(T_TelemetryFrame));
    
    printf("===== Minimal Test =====\n");
    test_minimal();
    printf("\n");

    printf("===== Generated Test Vector Test =====\n");
    test_with_generated_data();
    printf("\n");
    
    printf("===== Hardcoded Data Test =====\n");
    test_decoding();
    printf("\n");
    
    printf("===== Test Summary =====\n");
    printf("1. Minimal test: Tests basic encode/decode functionality\n");
    printf("2. Generated data test: Tests with fresh, properly initialized data\n");
    printf("3. Hardcoded data test: Tests with your original test vector\n");
    printf("\nIf the hardcoded data test fails but others pass,\n");
    printf("it means your test vector doesn't match the current ASN.1 schema.\n");
    
    return 0;
}
