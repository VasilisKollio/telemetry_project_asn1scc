# ASN.1 Satellite Telemetry Project - Docker Workflow

This document explains the Docker-based workflow for building and running the ASN.1 satellite telemetry decoder.

## Prerequisites

- Docker installed on your system
- Clone repositories: `asn1scc` and `telemetry_project_asn1scc` directories in your home folder
- ASN1SCC Repository: [https://github.com/esa/asn1scc.git](https://github.com/esa/asn1scc.git)
  
## Quick Start

### 1. Starting the Docker container

```bash
docker run -it --rm -v ~/asn1scc:/root/asn1scc -v ~/telemetry_project_asn1scc:/root/telemetry_project_asn1scc asn1scc /bin/bash
```

**Command Explanation:**
```
-it              # Interactive terminal session
--rm             # Automatically remove container after exit
-v ~/asn1scc...  # Mounts host's ASN1SCC compiler to container
-v ~/telemetry... # Mounts project directory to container
asn1scc          # Docker image name
/bin/bash        # Starts Bash shell
```

### 2. Building the Project

```bash
cd /root/telemetry_project_asn1scc
./build.sh
```

**Expected Build Output:**
```
=== Cleaning and setting up directories ===
=== Compiling ASN.1 with T_ prefix ===
=== Verifying generated files ===
=== Copying runtime extensions ===
'/root/telemetry_project_asn1scc/src/asn1crt_mempool.c' -> '/root/.../generated/asn1crt_mempool.c'
[... 3 more files copied ...]
=== Compiling program ===
=== BUILD SUCCESSFUL ===
Type prefixes are active (T_ prefix for types)
Run with: ./telemetry_program
```

**Build Process Steps:**
1. **Directory Setup**
   - Cleans and recreates generated directory
   
2. **ASN.1 Compilation**
   - Uses asn1scc with flags:
     -c -uPER -typePrefix T_ -renamePolicy 1
     
3. **Runtime Setup**
   - Copies required runtime files:
     - asn1crt_mempool.[c|h]
     - asn1crt_stream.[c|h]
     
4. **Final Compilation**
   - Uses GCC with:
     -Wall -Wextra -I/generated -I/src -lm

### 3. Running the Program

```bash
./telemetry_program
```

**Sample Program Output:**
```
===== Telemetry Decoder Test =====
Test Data (20 bytes):
00 00 00 01 03 e8 00 00
01 80 13 88 0d ac 09 c4
01 01 2c 80

=== DECODE SUCCESS ===
Bytes processed: 15/20
Frame count: 0
Payload type: 1
UNEXPECTED payload type! Check CHOICE tags.
First payload byte: 0x00
```

## Volume Mount Structure

| Host Path | Container Path |
|-----------|---------------|
| ~/asn1scc | /root/asn1scc |
| ~/telemetry_project_asn1scc | /root/telemetry_project_asn1scc |
