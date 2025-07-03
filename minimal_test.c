// minimal_test_corrected.c - Now with proper structure knowledge
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "generated/satellite.h"
#include "generated/asn1crt.h"

int main() {
    printf("===== PROPER STRUCTURE INITIALIZATION =====\n");
    
    T_TelemetryFrame frame_original, frame_decoded;
    unsigned char buffer[1024];
    
    // Zero out structures first
    memset(&frame_original, 0, sizeof(T_TelemetryFrame));
    memset(&frame_decoded, 0, sizeof(T_TelemetryFrame));
    memset(buffer, 0, sizeof(buffer));
    
    printf("Structure size: %zu bytes\n", sizeof(T_TelemetryFrame));
    printf("Structures zeroed out\n");
    
    // CRITICAL: Use the proper initialization functions
    printf("=== USING PROPER INITIALIZATION ===\n");
    T_TelemetryFrame_Initialize(&frame_original);
    printf("T_TelemetryFrame_Initialize() called\n");
    
    // Verify constraint validity before encoding
    int constraint_error;
    if (!T_TelemetryFrame_IsConstraintValid(&frame_original, &constraint_error)) {
        printf("WARNING: Initial constraints invalid, error: %d\n", constraint_error);
        printf("This is expected for default-initialized structure\n");
    } else {
        printf("Initial constraints are valid\n");
    }
    
    // Let's try to set some basic valid values
    printf("=== SETTING BASIC VALUES ===\n");
    
    // Initialize timestamp with valid values
    frame_original.header.timestamp.seconds = 1000000;  // Valid range: 0..4294967295
    frame_original.header.timestamp.subseconds = 500;   // Valid range: 0..1000
    printf("Set timestamp: %u.%u\n", 
           frame_original.header.timestamp.seconds,
           frame_original.header.timestamp.subseconds);
    
    // Initialize frame header fields
    frame_original.header.frameType = 1;    // Valid range: 0..255
    frame_original.header.frameCount = 42;  // Valid range: 0..65535
    printf("Set frameType: %u, frameCount: %u\n",
           frame_original.header.frameType,
           frame_original.header.frameCount);
    
    // Check constraints again
    if (!T_TelemetryFrame_IsConstraintValid(&frame_original, &constraint_error)) {
        printf("ERROR: Constraints still invalid after initialization, error: %d\n", constraint_error);
        
        // Check individual components
        int header_error;
        if (!T_FrameHeader_IsConstraintValid(&frame_original.header, &header_error)) {
            printf("Header constraints invalid, error: %d\n", header_error);
            
            // Check timestamp
            int ts_error;
            if (!T_TimeStamp_IsConstraintValid(&frame_original.header.timestamp, &ts_error)) {
                printf("Timestamp constraints invalid, error: %d\n", ts_error);
            }
        }
        
        int payload_error;
        if (!T_TelemetryPayload_IsConstraintValid(&frame_original.payload, &payload_error)) {
            printf("Payload constraints invalid, error: %d\n", payload_error);
        }
        
        printf("Attempting encoding anyway...\n");
    } else {
        printf("All constraints are now valid!\n");
    }
    
    printf("=== ENCODING TEST ===\n");
    
    // Initialize stream
    BitStream stream;
    BitStream_Init(&stream, buffer, sizeof(buffer));
    
    // Try encoding
    int encoding_result;
    if (!T_TelemetryFrame_Encode(&frame_original, &stream, &encoding_result, TRUE)) {
        printf("ERROR: Encoding failed with result: %d\n", encoding_result);
        
        // Map common error codes
        switch(encoding_result) {
            case 131:
                printf("Error 131: Likely constraint violation or missing required field\n");
                break;
            case 147: // ERR_UPER_ENCODE_TELEMETRYFRAME
                printf("Error 147: TelemetryFrame encoding error\n");
                break;
            case 137: // ERR_UPER_ENCODE_TELEMETRYFRAME_HEADER
                printf("Error 137: TelemetryFrame header encoding error\n");
                break;
            case 142: // ERR_UPER_ENCODE_TELEMETRYFRAME_PAYLOAD
                printf("Error 142: TelemetryFrame payload encoding error\n");
                break;
            default:
                printf("Unknown encoding error\n");
                break;
        }
        return 1;
    }
    
    int encoded_length = BitStream_GetLength(&stream);
    printf("Encoding successful: %d bytes\n", encoded_length);
    
    // Print encoded data
    printf("Encoded bytes: ");
    for (int i = 0; i < encoded_length && i < 50; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
    
    printf("=== DECODING TEST ===\n");
    
    // Decode
    BitStream decode_stream;
    BitStream_AttachBuffer(&decode_stream, buffer, encoded_length);
    
    int decoding_result;
    if (!T_TelemetryFrame_Decode(&frame_decoded, &decode_stream, &decoding_result)) {
        printf("ERROR: Decoding failed with result: %d\n", decoding_result);
        
        // Map decoding errors
        switch(decoding_result) {
            case 73:
                printf("Error 73: The original decoding error we were investigating\n");
                break;
            case 148: // ERR_UPER_DECODE_TELEMETRYFRAME
                printf("Error 148: TelemetryFrame decoding error\n");
                break;
            case 138: // ERR_UPER_DECODE_TELEMETRYFRAME_HEADER
                printf("Error 138: TelemetryFrame header decoding error\n");
                break;
            case 143: // ERR_UPER_DECODE_TELEMETRYFRAME_PAYLOAD
                printf("Error 143: TelemetryFrame payload decoding error\n");
                break;
            default:
                printf("Unknown decoding error\n");
                break;
        }
        
        printf("Stream position: bit %d, byte %ld\n", 
               decode_stream.currentBit, decode_stream.currentByte);
        return 1;
    }
    
    printf("Decoding successful!\n");
    
    // Verify data
    printf("=== VERIFICATION ===\n");
    printf("Original timestamp: %u.%u\n", 
           frame_original.header.timestamp.seconds,
           frame_original.header.timestamp.subseconds);
    printf("Decoded timestamp: %u.%u\n", 
           frame_decoded.header.timestamp.seconds,
           frame_decoded.header.timestamp.subseconds);
    
    printf("Original frame: type=%u, count=%u\n",
           frame_original.header.frameType,
           frame_original.header.frameCount);
    printf("Decoded frame: type=%u, count=%u\n",
           frame_decoded.header.frameType,
           frame_decoded.header.frameCount);
    
    printf("===== TEST COMPLETE =====\n");
    return 0;
}
