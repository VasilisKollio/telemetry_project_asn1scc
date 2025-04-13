#!/bin/bash
set -e

# Use absolute paths
PROJECT_DIR=$(realpath .)
GENERATED_DIR="${PROJECT_DIR}/generated"
SRC_DIR="${PROJECT_DIR}/src"
TESTS_DIR="${PROJECT_DIR}/tests"

# Clean and create directory
echo "=== Cleaning and setting up directories ==="
rm -rf "${GENERATED_DIR}"
mkdir -p "${GENERATED_DIR}"

# 1. Compile ASN.1 definitions with type prefix
echo "=== Compiling ASN.1 with T_ prefix ==="
asn1scc -c -uPER -typePrefix T_ -renamePolicy 1 -o "${GENERATED_DIR}" "${PROJECT_DIR}/examples/satellite.asn" || {
    echo "ASN1SCC compilation failed"
    exit 1
}

# Verify critical files were generated
echo "=== Verifying generated files ==="
[ -f "${GENERATED_DIR}/satellite.c" ] || {
    echo "Error: satellite.c not found in generated files:"
    ls -la "${GENERATED_DIR}"
    exit 1
}

# 2. Copy runtime extensions
echo "=== Copying runtime extensions ==="
cp -v "${SRC_DIR}/asn1crt_mempool.c" "${GENERATED_DIR}/"
cp -v "${SRC_DIR}/asn1crt_mempool.h" "${GENERATED_DIR}/"
cp -v "${SRC_DIR}/asn1crt_stream.c" "${GENERATED_DIR}/"
cp -v "${SRC_DIR}/asn1crt_stream.h" "${GENERATED_DIR}/"

# 3. Compile with explicit paths
echo "=== Compiling program ==="
gcc -Wall -Wextra \
    -I"${GENERATED_DIR}" \
    -I"${SRC_DIR}" \
    "${GENERATED_DIR}/asn1crt.c" \
    "${GENERATED_DIR}/asn1crt_encoding.c" \
    "${GENERATED_DIR}/asn1crt_encoding_uper.c" \
    "${GENERATED_DIR}/satellite.c" \
    "${GENERATED_DIR}/asn1crt_mempool.c" \
    "${GENERATED_DIR}/asn1crt_stream.c" \
    "${TESTS_DIR}/test_optimized_decoders.c" \
    -o "${PROJECT_DIR}/telemetry_program" \  
    -lm || {
    echo "Main Compilation failed"
    exit 1
}

echo "=== BUILD SUCCESSFUL ==="
echo "Type prefixes are active (T_ prefix for types)"
echo "Run main program with: ./telemetry_program"

