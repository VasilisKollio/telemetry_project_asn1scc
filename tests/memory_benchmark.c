#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "asn1crt.h"
#include "asn1crt_mempool.h"
#include "satellite.h"

// Function to generate test telemetry data
void generate_test_frame(byte* buffer, size_t* size) {
    // Create a sample telemetry frame
    T_TelemetryFrame frame;
    T_TelemetryFrame_Initialize(&frame);
    
    // Set header values
    frame.header.timestamp.seconds = 1;
    frame.header.timestamp.subseconds = 1000;
    frame.header.frameType = 0;
    frame.header.frameCount = 1;
    
    // Set payload (housekeeping data)
    frame.payload.kind = housekeeping_PRESENT;
    frame.payload.u.housekeeping.voltages.mainBus = 5000;
    frame.payload.u.housekeeping.voltages.payload = 3500;
    frame.payload.u.housekeeping.voltages.comms = 2500;
    frame.payload.u.housekeeping.temperature.nCount = 1;
    frame.payload.u.housekeeping.temperature.arr[0] = 30;
    memset(&frame.payload.u.housekeeping.status, 0, sizeof(frame.payload.u.housekeeping.status));
    
    // Encode the frame
    BitStream bs;
    BitStream_Init(&bs, buffer, 4096);
    int errCode;
    
    if (!T_TelemetryFrame_Encode(&frame, &bs, &errCode, TRUE)) {
        printf("Failed to encode test frame: error %d\n", errCode);
        *size = 0;
        return;
    }
    
    *size = bs.currentByte;
}

// Standard implementation (using malloc/free)
void benchmark_standard(int iterations) {
    clock_t start_time = clock();
    size_t total_memory = 0;
    
    // Generate test data
    byte test_data[4096];
    size_t data_size;
    generate_test_frame(test_data, &data_size);
    
    printf("Test data size: %zu bytes\n", data_size);
    
    for (int i = 0; i < iterations; i++) {
        // Allocate memory for frame
        T_TelemetryFrame* frame = (T_TelemetryFrame*)malloc(sizeof(T_TelemetryFrame));
        total_memory += sizeof(T_TelemetryFrame);
        
        // Initialize structure
        T_TelemetryFrame_Initialize(frame);
        
        // Create BitStream
        BitStream bs;
        BitStream_Init(&bs, test_data, data_size);
        
        // Decode frame
        int errCode;
        T_TelemetryFrame_Decode(frame, &bs, &errCode);
        
        // Free memory
        free(frame);
    }
    
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("\nStandard Implementation (malloc/free):\n");
    printf("Iterations: %d\n", iterations);
    printf("Total memory allocated: %zu bytes\n", total_memory);
    printf("Average memory per frame: %.2f bytes\n", (float)total_memory / iterations);
    printf("Processing time: %.4f seconds\n", elapsed_time);
    printf("Frames per second: %.2f\n", iterations / elapsed_time);
}

// Optimized implementation (using memory pool)
void benchmark_mempool(int iterations) {
    clock_t start_time = clock();
    
    // Generate test data
    byte test_data[4096];
    size_t data_size;
    generate_test_frame(test_data, &data_size);
    
    // Create and initialize memory pool
    size_t pool_size = sizeof(T_TelemetryFrame) * iterations;
    byte* pool_buffer = (byte*)malloc(pool_size);
    if (!pool_buffer) {
        printf("Failed to allocate memory pool\n");
        return;
    }
    
    MemPool pool;
    MemPool_Init(&pool, pool_buffer, pool_size);
    
    for (int i = 0; i < iterations; i++) {
        // Allocate from pool
        T_TelemetryFrame* frame = (T_TelemetryFrame*)MemPool_Alloc(&pool, sizeof(T_TelemetryFrame));
        if (!frame) {
            printf("Memory pool exhausted after %d iterations\n", i);
            break;
        }
        
        // Initialize structure
        T_TelemetryFrame_Initialize(frame);
        
        // Create BitStream
        BitStream bs;
        BitStream_Init(&bs, test_data, data_size);
        
        // Decode frame
        int errCode;
        T_TelemetryFrame_Decode(frame, &bs, &errCode);
        
        // No need to free individual frames
    }
    
    size_t used_memory = pool.used;
    
    // Free pool
    free(pool_buffer);
    
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("\nOptimized Implementation (memory pool):\n");
    printf("Iterations: %d\n", iterations);
    printf("Total memory used: %zu bytes\n", used_memory);
    printf("Average memory per frame: %.2f bytes\n", (float)used_memory / iterations);
    printf("Processing time: %.4f seconds\n", elapsed_time);
    printf("Frames per second: %.2f\n", iterations / elapsed_time);
}

// Calculate and display percentage improvements
void calculate_improvements(int iterations) {
    // For memory
    clock_t start_time = clock();
    
    // Standard
    size_t std_memory = sizeof(T_TelemetryFrame) * iterations;
    
    // Memory pool
    byte test_data[4096];
    size_t data_size;
    generate_test_frame(test_data, &data_size);
    
    size_t pool_size = sizeof(T_TelemetryFrame) * iterations;
    byte* pool_buffer = (byte*)malloc(pool_size);
    MemPool pool;
    MemPool_Init(&pool, pool_buffer, pool_size);
    
    for (int i = 0; i < iterations; i++) {
        T_TelemetryFrame* frame = (T_TelemetryFrame*)MemPool_Alloc(&pool, sizeof(T_TelemetryFrame));
        T_TelemetryFrame_Initialize(frame);
        
        BitStream bs;
        BitStream_Init(&bs, test_data, data_size);
        
        int errCode;
        T_TelemetryFrame_Decode(frame, &bs, &errCode);
    }
    
    size_t pool_memory = pool.used;
    free(pool_buffer);
    
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    // Calculate percentage reductions
    float memory_reduction = ((float)(std_memory - pool_memory) / std_memory) * 100.0f;
    
    printf("\nPerformance Improvements:\n");
    printf("Memory usage reduction: %.2f%%\n", memory_reduction);
}

int main(int argc, char** argv) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    printf("===== ASN.1 Telemetry Memory Benchmark =====\n");
    printf("Running with %d iterations\n", iterations);
    
    benchmark_standard(iterations);
    benchmark_mempool(iterations);
    calculate_improvements(iterations);
    
    return 0;
}
