#!/bin/bash
set -e

# Enhanced build script for ASN1SCC telemetry optimization thesis
# Incorporates optimization techniques and comprehensive validation

# Use absolute paths for robustness
PROJECT_DIR=$(realpath .)
GENERATED_DIR="${PROJECT_DIR}/generated"
SRC_DIR="${PROJECT_DIR}/src"
TESTS_DIR="${PROJECT_DIR}/tests"
EXAMPLES_DIR="${PROJECT_DIR}/examples"

# Build configuration
OPTIMIZATION_LEVEL="-O2"
COMPILER_FLAGS="-Wall -Wextra -Wno-unused-parameter"
TARGET_ARCH=${TARGET_ARCH:-"native"}

echo "=========================================="
echo "ASN1SCC Telemetry Optimization Build System"
echo "=========================================="
echo "Project Directory: ${PROJECT_DIR}"
echo "Target Architecture: ${TARGET_ARCH}"
echo "Optimization Level: ${OPTIMIZATION_LEVEL}"

# Clean and create directories
echo "=== Cleaning and setting up directories ==="
rm -rf "${GENERATED_DIR}"
mkdir -p "${GENERATED_DIR}"

# Validate source files exist
echo "=== Validating source files ==="
ASN1_SCHEMA="${EXAMPLES_DIR}/satellite.asn"
if [ ! -f "${ASN1_SCHEMA}" ]; then
    echo "Error: ASN.1 schema not found: ${ASN1_SCHEMA}"
    exit 1
fi

# Check for required source files
REQUIRED_SRC_FILES=(
    "${SRC_DIR}/asn1crt_mempool.c"
    "${SRC_DIR}/asn1crt_mempool.h" 
    "${SRC_DIR}/asn1crt_stream.c"
    "${SRC_DIR}/asn1crt_stream.h"
)

for file in "${REQUIRED_SRC_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "Error: Required source file not found: $file"
        exit 1
    fi
done

# 1. Compile ASN.1 definitions with optimization-friendly settings
echo "=== Compiling ASN.1 with optimization settings ==="
echo "Using ASN1SCC with uPER encoding and type prefixes..."

asn1scc -c -uPER \
        -typePrefix T_ \
        -renamePolicy 1 \
        -o "${GENERATED_DIR}" \
        "${ASN1_SCHEMA}" || {
    echo "ERROR: ASN1SCC compilation failed"
    echo "Check that ASN1SCC is properly installed and in PATH"
    exit 1
}

# Verify critical files were generated
echo "=== Verifying generated files ==="
REQUIRED_GENERATED_FILES=(
    "${GENERATED_DIR}/satellite.c"
    "${GENERATED_DIR}/satellite.h"
    "${GENERATED_DIR}/asn1crt.c"
    "${GENERATED_DIR}/asn1crt.h"
    "${GENERATED_DIR}/asn1crt_encoding.c"
    "${GENERATED_DIR}/asn1crt_encoding_uper.c"
)

for file in "${REQUIRED_GENERATED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "Error: Expected generated file not found: $file"
        echo "Generated files:"
        ls -la "${GENERATED_DIR}"
        exit 1
    fi
done

echo "Generated files verified successfully."

# 2. Copy optimization runtime extensions
echo "=== Copying runtime extensions ==="
echo "Installing memory pool optimization..."
cp -v "${SRC_DIR}/asn1crt_mempool.c" "${GENERATED_DIR}/"
cp -v "${SRC_DIR}/asn1crt_mempool.h" "${GENERATED_DIR}/"

echo "Installing stream optimization..."
cp -v "${SRC_DIR}/asn1crt_stream.c" "${GENERATED_DIR}/"
cp -v "${SRC_DIR}/asn1crt_stream.h" "${GENERATED_DIR}/"

# 3. Compile main telemetry program with optimizations
echo "=== Compiling main program ==="
echo "Building optimized telemetry program..."

# Define source files for main program
MAIN_SOURCES=(
    "${GENERATED_DIR}/asn1crt.c"
    "${GENERATED_DIR}/asn1crt_encoding.c"
    "${GENERATED_DIR}/asn1crt_encoding_uper.c"
    "${GENERATED_DIR}/satellite.c"
    "${GENERATED_DIR}/asn1crt_mempool.c"
    "${GENERATED_DIR}/asn1crt_stream.c"
    "${TESTS_DIR}/test_optimized_decoders.c"
)

# Check test file exists
if [ ! -f "${TESTS_DIR}/test_optimized_decoders.c" ]; then
    echo "Warning: Main test file not found: ${TESTS_DIR}/test_optimized_decoders.c"
    echo "Available test files:"
    ls -la "${TESTS_DIR}/" || echo "Tests directory not found"
    exit 1
fi

gcc ${COMPILER_FLAGS} ${OPTIMIZATION_LEVEL} \
    -I"${GENERATED_DIR}" \
    -I"${SRC_DIR}" \
    "${MAIN_SOURCES[@]}" \
    -o "${PROJECT_DIR}/telemetry_program" \
    -lm || {
    echo "ERROR: Main program compilation failed"
    exit 1
}

echo "Main program compiled successfully: ./telemetry_program"

# 4. Compile memory benchmark if available
echo "=== Compiling memory benchmark ==="
if [ -f "${TESTS_DIR}/memory_benchmark.c" ]; then
    echo "Building memory performance benchmark..."
    
    BENCHMARK_SOURCES=(
        "${GENERATED_DIR}/asn1crt.c"
        "${GENERATED_DIR}/asn1crt_encoding.c"
        "${GENERATED_DIR}/asn1crt_encoding_uper.c"
        "${GENERATED_DIR}/satellite.c"
        "${GENERATED_DIR}/asn1crt_mempool.c"
        "${GENERATED_DIR}/asn1crt_stream.c"
        "${TESTS_DIR}/memory_benchmark.c"
    )
    
    gcc ${COMPILER_FLAGS} ${OPTIMIZATION_LEVEL} \
        -I"${GENERATED_DIR}" \
        -I"${SRC_DIR}" \
        "${BENCHMARK_SOURCES[@]}" \
        -o "${PROJECT_DIR}/memory_benchmark" \
        -lm || {
        echo "Warning: Memory benchmark compilation failed"
    }
    
    if [ -f "${PROJECT_DIR}/memory_benchmark" ]; then
        echo "Memory benchmark compiled successfully: ./memory_benchmark"
    fi
else
    echo "Memory benchmark source not found, skipping..."
fi

# 5. Generate build information
echo "=== Generating build information ==="
BUILD_INFO="${PROJECT_DIR}/build_info.txt"
cat > "${BUILD_INFO}" << EOF
ASN1SCC Telemetry Optimization Build Information
================================================
Build Date: $(date)
Build Host: $(hostname)
Compiler: $(gcc --version | head -n1)
ASN1SCC Version: $(asn1scc --version 2>/dev/null || echo "Version check failed")
Project Directory: ${PROJECT_DIR}
Optimization Level: ${OPTIMIZATION_LEVEL}
Target Architecture: ${TARGET_ARCH}

Generated Files:
$(ls -la "${GENERATED_DIR}")

Build Status: SUCCESS
EOF

echo "Build information saved to: ${BUILD_INFO}"

# 6. Validation and summary
echo "=========================================="
echo "=== BUILD SUCCESSFUL ==="
echo "=========================================="
echo ""
echo "Optimization Features Enabled:"
echo "  ✓ Type prefixes active (T_ prefix for types)"
echo "  ✓ Memory pool optimization integrated"
echo "  ✓ Enhanced BitStream operations"
echo "  ✓ uPER encoding optimization"
echo ""
echo "Executables Generated:"
[ -f "${PROJECT_DIR}/telemetry_program" ] && echo "  ✓ ./telemetry_program (main test program)"
[ -f "${PROJECT_DIR}/memory_benchmark" ] && echo "  ✓ ./memory_benchmark [iterations] (performance benchmark)"
echo ""
echo "Usage Instructions:"
echo "  Run comprehensive tests:     ./telemetry_program"
[ -f "${PROJECT_DIR}/memory_benchmark" ] && echo "  Run performance benchmark:   ./memory_benchmark 1000"
[ -f "${PROJECT_DIR}/memory_benchmark" ] && echo "  Run stability test:          ./memory_benchmark 1000 30"
echo ""
echo "For thesis validation, run both programs and document results."
echo "Expected: Error-free encoding/decoding with performance > 70M ops/sec"
