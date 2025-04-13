#!/bin/bash
set -e 

#Define paths
PROJECT_DIR=$(pwd)
GENERATED_DIR="${PROJECT_DIR}/generated"
SRC_DIR="${PROJECT_DIR}/src"
TESTS_DIR="${PROJECT_DIR}/tests"

echo "Checking directories exist.."
ls -la "${GENERATED_DIR}"
ls -la "${SRC_DIR}"
ls -la "${TESTS_DIR}"

echo "Compiling main program..."
gcc -Wall -I"${GENERATED_DIR}" -I"${SRC_DIR}" \
   "${GENERATED_DIR}/asn1crt.c" \
   "${GENERATED_DIR}/asn1crt_encoding.c" \
   "${GENERATED_DIR}/asn1crt_encoding_uper.c" \
   "${GENERATED_DIR}/satellite.c" \
   "${GENERATED_DIR}/asn1crt_mempool.c" \
   "${GENERATED_DIR}/asn1crt_stream.c" \
   "${TESTS_DIR}/test_optimized_decoders.c" \
   -o "${PROJECT_DIR}/telemetry_program" -lm

echo "Main program compilation successful!"

# Now compile the memory benchmark if file exists
if [ -f "${TESTS_DIR}/memory_benchmark.c" ]; then
    echo "Compiling memory benchmark..."
    gcc -Wall -I"${GENERATED_DIR}" -I"${SRC_DIR}" \
       "${GENERATED_DIR}/asn1crt.c" \
       "${GENERATED_DIR}/asn1crt_encoding.c" \
       "${GENERATED_DIR}/asn1crt_encoding_uper.c" \
       "${GENERATED_DIR}/satellite.c" \
       "${GENERATED_DIR}/asn1crt_mempool.c" \
       "${GENERATED_DIR}/asn1crt_stream.c" \
       "${TESTS_DIR}/memory_benchmark.c" \
       -o "${PROJECT_DIR}/memory_benchmark" -lm
    
    echo "Memory benchmark compilation successful!"
else
    echo "Skipping memory benchmark - file not found: ${TESTS_DIR}/memory_benchmark.c"
fi

echo "=== BUILD SUCCESSFUL ==="
echo "Run main program with: ./telemetry_program"

if [ -f "${PROJECT_DIR}/memory_benchmark" ]; then
    echo "Run memory benchmark with: ./memory_benchmark [iterations]"
fi
