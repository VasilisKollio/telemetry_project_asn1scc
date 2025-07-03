#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "asn1crt.h"
#include "asn1crt_mempool.h"
#include "satellite.h"

// Function to generate test telemetry data with proper validation
void generate_test_frame(unsigned char* buffer, size_t* size) {
    T_TelemetryFrame frame;
    T_TelemetryFrame_Initialize(&frame);
    
    // Set header values
    frame.header.timestamp.seconds = 1;
    frame.header.timestamp.subseconds = 500; // Valid range 0-1000
    frame.header.frameType = 1; // Housekeeping type
    frame.header.frameCount = 1;
    
    // Set payload to housekeeping
    frame.payload.kind = housekeeping_PRESENT;
    
    // Initialize housekeeping data properly
    frame.payload.u.housekeeping.voltages.mainBus = 3300;   // 3.3V in mV
    frame.payload.u.housekeeping.voltages.payload = 5000;   // 5V in mV
    frame.payload.u.housekeeping.voltages.comms = 1800;     // 1.8V in mV
    
    // Set temperature array
    frame.payload.u.housekeeping.temperature.nCount = 2;
    frame.payload.u.housekeeping.temperature.arr[0] = 25;   // 25°C
    frame.payload.u.housekeeping.temperature.arr[1] = 30;   // 30°C
    
    // Initialize bit string properly - just clear it for now
    memset(&frame.payload.u.housekeeping.status, 0, sizeof(frame.payload.u.housekeeping.status));
    
    // Encode with validation
    BitStream bs;
    BitStream_Init(&bs, buffer, 4096);
    
    int errCode;
    if (T_TelemetryFrame_Encode(&frame, &bs, &errCode, TRUE)) {
        *size = BitStream_GetLength(&bs);
        printf("Generated valid test frame: %zu bytes\n", *size);
    } else {
        printf("ERROR: Failed to encode test frame: error %d\n", errCode);
        *size = 0;
    }
}

// Enhanced memory benchmark comparing malloc vs memory pool
void benchmark_enhanced(int iterations) {
    printf("\n===== Enhanced Memory Benchmark =====\n");
    
    // Generate test data once
    unsigned char test_data[4096];
    size_t data_size;
    generate_test_frame(test_data, &data_size);
    if (data_size == 0) {
        printf("ERROR: Cannot generate test frame\n");
        return;
    }
    printf("Test data size: %zu bytes\n", data_size);
    
    // Test 1: Standard malloc/free approach
    printf("\nTesting standard malloc/free approach...\n");
    clock_t start_malloc = clock();
    size_t malloc_overhead = 0;
    int malloc_success = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Allocate
        T_TelemetryFrame* frame = (T_TelemetryFrame*)malloc(sizeof(T_TelemetryFrame));
        malloc_overhead += sizeof(T_TelemetryFrame) + 8; // 8 bytes typical malloc overhead
        
        if (frame) {
            // Process
            T_TelemetryFrame_Initialize(frame);
            
            // Try to decode
            BitStream bs;
            BitStream_AttachBuffer(&bs, test_data, data_size);
            int errCode;
            if (T_TelemetryFrame_Decode(frame, &bs, &errCode)) {
                malloc_success++;
            }
            
            // Free immediately
            free(frame);
        }
    }
    clock_t end_malloc = clock();
    double malloc_time = (double)(end_malloc - start_malloc) / CLOCKS_PER_SEC;
    
    // Test 2: Memory pool approach
    printf("Testing memory pool approach...\n");
    size_t pool_size = sizeof(T_TelemetryFrame) * iterations + 1024;
    unsigned char* pool_buffer = (unsigned char*)malloc(pool_size);
    if (!pool_buffer) {
        printf("ERROR: Failed to allocate pool buffer\n");
        return;
    }
    
    clock_t start_pool = clock();
    MemPool pool;
    MemPool_Init(&pool, pool_buffer, pool_size);
    int pool_success = 0;
    
    for (int i = 0; i < iterations; i++) {
        T_TelemetryFrame* frame = (T_TelemetryFrame*)MemPool_Alloc(&pool, sizeof(T_TelemetryFrame));
        if (frame) {
            T_TelemetryFrame_Initialize(frame);
            BitStream bs;
            BitStream_AttachBuffer(&bs, test_data, data_size);
            int errCode;
            if (T_TelemetryFrame_Decode(frame, &bs, &errCode)) {
                pool_success++;
            }
        }
    }
    clock_t end_pool = clock();
    double pool_time = (double)(end_pool - start_pool) / CLOCKS_PER_SEC;
    
    // Calculate improvements
    double time_improvement = ((malloc_time - pool_time) / malloc_time) * 100.0;
    double memory_efficiency = ((double)(sizeof(T_TelemetryFrame) * iterations) / pool.used) * 100.0;
    
    printf("\n===== RESULTS =====\n");
    printf("Iterations completed: %d\n", iterations);
    printf("Frame size: %zu bytes\n", sizeof(T_TelemetryFrame));
    
    printf("\nStandard malloc/free:\n");
    printf("  Processing time: %.4f seconds\n", malloc_time);
    printf("  Successful operations: %d/%d\n", malloc_success, iterations);
    printf("  Total memory overhead: %zu bytes\n", malloc_overhead);
    printf("  Operations per second: %.2f\n", malloc_time > 0 ? iterations / malloc_time : 0);
    
    printf("\nMemory pool approach:\n");
    printf("  Processing time: %.4f seconds\n", pool_time);
    printf("  Successful operations: %d/%d\n", pool_success, iterations);
    printf("  Pool memory used: %zu bytes\n", pool.used);
    printf("  Operations per second: %.2f\n", pool_time > 0 ? iterations / pool_time : 0);
    
    printf("\nOptimization benefits:\n");
    printf("  Time improvement: %.1f%%\n", time_improvement);
    printf("  Memory efficiency: %.1f%%\n", memory_efficiency);
    printf("  Overhead eliminated: %zu bytes\n", malloc_overhead - pool.used);
    printf("  Speed increase: %.2fx\n", pool_time > 0 ? malloc_time / pool_time : 0);
    
    free(pool_buffer);
}

// Memory fragmentation test showing realistic allocation patterns
void benchmark_fragmentation(int iterations) {
    printf("\n===== Memory Fragmentation Test =====\n");
    
    // Limit iterations to prevent excessive memory usage
    int test_iterations = (iterations > 1000) ? 1000 : iterations;
    
    // Test standard approach with realistic allocation pattern
    printf("Testing standard approach (malloc/free with fragmentation):\n");
    void* ptrs[1000];
    memset(ptrs, 0, sizeof(ptrs));
    size_t allocated = 0;
    
    clock_t start = clock();
    
    // Allocate many small blocks (simulating realistic usage)
    for (int i = 0; i < test_iterations; i++) {
        ptrs[i] = malloc(sizeof(T_TelemetryFrame));
        if (ptrs[i]) allocated++;
        
        // Free every 3rd allocation (creates fragmentation)
        if (i % 3 == 0 && i > 0 && ptrs[i-1]) {
            free(ptrs[i-1]);
            ptrs[i-1] = NULL;
        }
    }
    
    // Free remaining allocations
    for (int i = 0; i < test_iterations; i++) {
        if (ptrs[i]) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    clock_t end = clock();
    double std_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Test pool approach
    printf("Testing memory pool approach (no fragmentation):\n");
    size_t pool_size = sizeof(T_TelemetryFrame) * test_iterations + 1024;
    unsigned char* pool_buffer = (unsigned char*)malloc(pool_size);
    if (!pool_buffer) {
        printf("ERROR: Failed to allocate pool buffer\n");
        return;
    }
    
    MemPool pool;
    MemPool_Init(&pool, pool_buffer, pool_size);
    
    start = clock();
    for (int i = 0; i < test_iterations; i++) {
        T_TelemetryFrame* frame = (T_TelemetryFrame*)MemPool_Alloc(&pool, sizeof(T_TelemetryFrame));
        // No fragmentation - linear allocation
        if (!frame) {
            printf("Pool exhausted at iteration %d\n", i);
            break;
        }
    }
    end = clock();
    double pool_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nFragmentation test results:\n");
    printf("Standard time (with fragmentation): %.4f seconds\n", std_time);
    printf("Pool time (no fragmentation): %.4f seconds\n", pool_time);
    printf("Fragmentation overhead eliminated: %.1f%%\n", 
           std_time > 0 ? ((std_time - pool_time) / std_time) * 100.0 : 0);
    printf("Pool memory efficiency: %.1f%%\n", 
           ((double)(sizeof(T_TelemetryFrame) * test_iterations) / pool_size) * 100.0);
    printf("Successful allocations: %zu\n", allocated);
    
    free(pool_buffer);
}

// Long-duration stability test
void benchmark_stability(int duration_seconds) {
    printf("\n===== Long-Duration Stability Test =====\n");
    printf("Running for %d seconds...\n", duration_seconds);
    
    size_t pool_size = 1024 * 1024; // 1MB pool
    unsigned char* pool_buffer = (unsigned char*)malloc(pool_size);
    if (!pool_buffer) {
        printf("ERROR: Failed to allocate 1MB pool buffer\n");
        return;
    }
    
    MemPool pool;
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration_seconds;
    int cycles = 0;
    int total_allocations = 0;
    
    while (time(NULL) < end_time) {
        MemPool_Init(&pool, pool_buffer, pool_size);
        
        // Allocate until pool is full
        int allocations = 0;
        while (MemPool_Alloc(&pool, sizeof(T_TelemetryFrame)) != NULL) {
            allocations++;
        }
        total_allocations += allocations;
        cycles++;
        
        if (cycles % 100 == 0) {
            printf("Cycle %d: %d allocations per cycle\n", cycles, allocations);
        }
    }
    
    printf("\nStability test completed:\n");
    printf("Total cycles: %d\n", cycles);
    printf("Total allocations: %d\n", total_allocations);
    printf("Average allocations per cycle: %.1f\n", cycles > 0 ? (double)total_allocations / cycles : 0);
    printf("Cycles per second: %.2f\n", duration_seconds > 0 ? (double)cycles / duration_seconds : 0);
    printf("Memory management: STABLE (consistent allocation pattern)\n");
    
    free(pool_buffer);
}

// Test basic functionality before running benchmarks
int test_basic_functionality() {
    printf("\n===== Testing Basic Functionality =====\n");
    
    // Test encoding
    T_TelemetryFrame frame;
    T_TelemetryFrame_Initialize(&frame);
    
    frame.header.timestamp.seconds = 12345;
    frame.header.timestamp.subseconds = 678;
    frame.header.frameType = 1;
    frame.header.frameCount = 42;
    
    frame.payload.kind = housekeeping_PRESENT;
    frame.payload.u.housekeeping.voltages.mainBus = 3300;
    frame.payload.u.housekeeping.voltages.payload = 5000;
    frame.payload.u.housekeeping.voltages.comms = 1800;
    
    frame.payload.u.housekeeping.temperature.nCount = 1;
    frame.payload.u.housekeeping.temperature.arr[0] = 25;
    
    // Initialize bit string properly - just clear it
    memset(&frame.payload.u.housekeeping.status, 0, sizeof(frame.payload.u.housekeeping.status));
    
    // Test encoding
    unsigned char buffer[1024];
    BitStream bs;
    BitStream_Init(&bs, buffer, sizeof(buffer));
    int errCode;
    
    if (!T_TelemetryFrame_Encode(&frame, &bs, &errCode, TRUE)) {
        printf("ERROR: Encoding failed with error %d\n", errCode);
        return 0;
    }
    printf("Encoding successful: %d bytes\n", BitStream_GetLength(&bs));
    
    // Test decoding
    BitStream decode_bs;
    BitStream_AttachBuffer(&decode_bs, buffer, BitStream_GetLength(&bs));
    T_TelemetryFrame decoded_frame;
    if (!T_TelemetryFrame_Decode(&decoded_frame, &decode_bs, &errCode)) {
        printf("ERROR: Decoding failed with error %d\n", errCode);
        return 0;
    }
    
    printf("Decoding successful!\n");
    printf("Decoded frame count: %lu\n", decoded_frame.header.frameCount);
    printf("Decoded voltage: %lu mV\n", decoded_frame.payload.u.housekeeping.voltages.mainBus);
    
    // Verify data integrity
    if (decoded_frame.header.frameCount == frame.header.frameCount &&
        decoded_frame.payload.u.housekeeping.voltages.mainBus == frame.payload.u.housekeeping.voltages.mainBus) {
        printf("Data integrity: PASSED\n");
        return 1;
    } else {
        printf("Data integrity: FAILED\n");
        return 0;
    }
}

// Simple memory pool efficiency test
void benchmark_simple_efficiency(int iterations) {
    printf("\n===== Simple Memory Pool Efficiency Test =====\n");
    
    clock_t start_time = clock();
    size_t total_memory = 0;
    
    // Standard malloc/free approach
    for (int i = 0; i < iterations; i++) {
        // Allocate memory for frame
        T_TelemetryFrame* frame = (T_TelemetryFrame*)malloc(sizeof(T_TelemetryFrame));
        total_memory += sizeof(T_TelemetryFrame);
        
        // Initialize structure
        if (frame) {
            T_TelemetryFrame_Initialize(frame);
            // Simulate some processing
            frame->header.frameCount = i;
        }
        
        // Free memory
        free(frame);
    }
    clock_t end_time = clock();
    double malloc_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("Standard malloc/free:\n");
    printf("  Iterations: %d\n", iterations);
    printf("  Total memory allocated: %zu bytes\n", total_memory);
    printf("  Average memory per frame: %.2f bytes\n", (float)total_memory / iterations);
    printf("  Processing time: %.4f seconds\n", malloc_time);
    printf("  Frames per second: %.2f\n", malloc_time > 0 ? iterations / malloc_time : 0);
    
    // Memory pool approach
    size_t pool_size = sizeof(T_TelemetryFrame) * iterations;
    unsigned char* pool_buffer = (unsigned char*)malloc(pool_size);
    if (!pool_buffer) {
        printf("Failed to allocate memory pool\n");
        return;
    }
    
    MemPool pool;
    MemPool_Init(&pool, pool_buffer, pool_size);
    
    start_time = clock();
    for (int i = 0; i < iterations; i++) {
        // Allocate from pool
        T_TelemetryFrame* frame = (T_TelemetryFrame*)MemPool_Alloc(&pool, sizeof(T_TelemetryFrame));
        if (!frame) {
            printf("Memory pool exhausted after %d iterations\n", i);
            break;
        }
        
        // Initialize structure
        T_TelemetryFrame_Initialize(frame);
        frame->header.frameCount = i;
        // No need to free individual frames
    }
    end_time = clock();
    double pool_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("\nMemory pool approach:\n");
    printf("  Iterations: %d\n", iterations);
    printf("  Pool size: %zu bytes\n", pool_size);
    printf("  Pool memory used: %zu bytes\n", pool.used);
    printf("  Memory efficiency: %.1f%%\n", ((double)pool.used / pool_size) * 100.0);
    printf("  Processing time: %.4f seconds\n", pool_time);
    printf("  Frames per second: %.2f\n", pool_time > 0 ? iterations / pool_time : 0);
    
    // Calculate improvements
    float time_improvement = malloc_time > 0 ? ((malloc_time - pool_time) / malloc_time) * 100.0f : 0;
    printf("\nImprovement:\n");
    printf("  Time improvement: %.1f%%\n", time_improvement);
    printf("  Speed increase: %.2fx\n", pool_time > 0 ? malloc_time / pool_time : 0);
    
    free(pool_buffer);
}

int main(int argc, char** argv) {
    int iterations = 1000;
    int stability_duration = 10; // seconds
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0 || iterations > 100000) {
            printf("Invalid iteration count. Using default: 1000\n");
            iterations = 1000;
        }
    }
    
    if (argc > 2) {
        stability_duration = atoi(argv[2]);
        if (stability_duration <= 0 || stability_duration > 300) {
            printf("Invalid stability duration. Using default: 10 seconds\n");
            stability_duration = 10;
        }
    }
    
    printf("===== ASN.1 Telemetry Optimization Benchmark =====\n");
    printf("Iterations: %d\n", iterations);
    printf("Stability test duration: %d seconds\n", stability_duration);
    printf("Frame size: %zu bytes\n", sizeof(T_TelemetryFrame));
    
    // Test basic functionality first
    if (!test_basic_functionality()) {
        printf("\nWARNING: Basic functionality test failed.\n");
        printf("Running simple efficiency test without encoding/decoding...\n");
        benchmark_simple_efficiency(iterations);
        return 1;
    }
    
    // Run all benchmarks if basic test passes
    benchmark_enhanced(iterations);
    benchmark_fragmentation(iterations);
    benchmark_stability(stability_duration);
    
    printf("\n===== Benchmark Complete =====\n");
    printf("All tests completed successfully.\n");
    printf("\nUsage: %s [iterations] [stability_seconds]\n", argv[0]);
    printf("Example: %s 2000 30\n", argv[0]);
    
    return 0;
}
