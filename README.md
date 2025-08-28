# ASN.1 Satellite Telemetry Decoder
# Before we start 
## Run this command to start Docker Environment
 ```bash
 docker run -it --rm -v ~/asn1scc:/root/asn1scc -v ~/telemetry_project_asn1scc:/root/telemetry_project_asn1scc asn1scc /bin/bash
 ```
 ## Working Directory
 ```bash
 cd /root/telemetry_project_asn1scc
 ```
## Git Push - Pull
If any changes have been made in th docker environment and want to push, just use these commands
```bash
git remote set-url origin https://Token_name_check_Notion_notes@github.com/VasilisKollio/telemetry_project_asn1scc.git
git pull origin main
# Check what you're about to overwrite
git log --oneline origin/main

# Abort the current merge
git merge --abort

# Force push your changes
git push origin main --force
```

## Main Error I am/was encountering
ASN.1 telemetry decoder was failing with **Error 73** during decoding operations:
```
Encoding successful: 15 bytes
ERROR: Decoding failed with error 73
```

## Solution
Fixed by using proper ASN.1 initialization instead of manual memory clearing:

```c
// Wrong - causes Error 73
memset(&frame, 0, sizeof(T_TelemetryFrame));

// Correct - fixes Error 73
T_TelemetryFrame_Initialize(&frame);
```

## Results
-  Error 73 completely resolved
-  70+ million operations per second
-  100% data integrity
-  Automated build system

## Usage

### Build and run:
```bash
./build.sh
./telemetry_program
./memory_benchmark
```

### Test output:
```
===== Minimal Test =====
Minimal encode successful: 16 bytes
Minimal decode successful
Round-trip test: PASSED

===== Generated Test Vector Test =====
Encoding successful! Generated 17 bytes
Decoding successful!
Data integrity: PASSED
```

## Performance
- **73,365 cycles per second**
- **2.1+ billion operations completed**
- **956 consistent allocations per cycle**
- **Zero memory leaks**

## Key Files
- `build.sh` - Automated build script
- `telemetry_program` - Main test executable
- `memory_benchmark` - Performance testing
- `generated/` - ASN.1 generated files
- `tests/` - Test programs

## What Was Fixed
1. **Proper initialization** - Use `T_TelemetryFrame_Initialize()`
2. **Memory pool optimization** - Eliminated malloc/free overhead
3. **Automated build process** - One command builds everything
4. **Comprehensive testing** - Validates all functionality

## Technical Details
- **Frame size:** 1096 bytes
- **Encoding:** uPER (Unaligned Packed Encoding Rules)
- **Language:** C with ASN1SCC compiler
- **Platform:** Docker container

## Step-by-Step Process

### 1. Identified the Problem
- ASN.1 encoding worked (15 bytes output)
- Decoding failed consistently with Error 73
- Issue happened in all test programs

### 2. Analyzed the Structure
```bash
# Examined ASN.1 generated files
grep -A 20 "typedef.*T_TelemetryFrame" generated/satellite.h
grep -i "init" generated/satellite.h
```

Found:
- `T_TelemetryFrame` has complex structure (1096 bytes)
- Contains `header` and `payload` fields
- Proper initialization functions exist

### 3. Created Minimal Test
Created `minimal_test.c` to isolate the problem:
```c
T_TelemetryFrame frame;
T_TelemetryFrame_Initialize(&frame);  // Key fix
frame.header.timestamp.seconds = 1000000;
frame.header.frameCount = 42;
```

### 4. Tested the Fix
Before: `ERROR: Decoding failed with error 73`
After: `Encoding/Decoding successful!`

### 5. Applied to All Programs
Updated memory benchmark and test programs with proper initialization.

### 6. Verified Performance
Ran benchmarks and achieved 70+ million operations per second.

### 7. Automated Build
Fixed `build.sh` to compile everything with one command.

### 8. Final Validation
All tests pass:
- Minimal test: 16 bytes encode/decode
- Generated data: 17 bytes with full validation
- Performance: 73K+ cycles per second

ASN.1 Error 73 solved. System working at 70M+ ops/sec.
