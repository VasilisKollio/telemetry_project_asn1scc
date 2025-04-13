# ASN.1 Satellite Telemetry Project - Docker Workflow

This document explains the Docker-based workflow for building and running the ASN.1 satellite telemetry decoder.

## Prerequisites

- Docker installed on your system
- Clone repositories: `asn1scc` and `telemetry_project_asn1scc` directories in your home folder
- ASN1SCC Repository: [https://github.com/esa/asn1scc.git](https://github.com/esa/asn1scc.git)
-  **Important**: Edit the Dockerfile in the asn1scc repository to include necessary libraries and set up the PATH for the asn1scc command (see Dockerfile content below)
  
### Required Dockerfile Content

Replace the Dockerfile in the asn1scc repository with the following content:

```dockerfile
FROM mcr.microsoft.com/dotnet/sdk:7.0 AS build
RUN set -xe \
    && DEBIAN_FRONTEND=noninteractive apt-get update -y \
        && apt-get install -y libfontconfig libdbus-1-3 libx11-6 libx11-xcb-dev cppcheck htop \
            python3 python3-pip python3-distutils gcc g++ make nuget libgit2-dev libssl-dev git cmake mono-complete wget nano \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get purge --auto-remove \
    && apt-get clean

# this SHELL command is needed to allow using source
SHELL ["/bin/bash", "-c"]

# Install dependencies for scala backend
RUN apt-get update -y \
        && apt-get install -y curl wget unzip zip \
        && curl -s "https://get.sdkman.io" | bash \
        && chmod a+x "$HOME/.sdkman/bin/sdkman-init.sh" \
        && source "$HOME/.sdkman/bin/sdkman-init.sh" \
        && sdk install java 17.0.9-oracle \
        && sdk install scala 3.3.0 \
        && sdk install sbt 1.9.0

# Install GNAT AND SPARK from AdaCore
WORKDIR /gnat_tmp/

# The ADD instruction will always download the file and the cache will be invalidated if the checksum of the file no longer matches
# On the other hand, the RUN instruction will not invalidate the cache unless its text changes.
# So if the remote file is updated, you won't get it. Docker will use the cached layer.
# In our case, the gnat-2021-20210519-x86_64-linux-bin will not change. So, it is preferable to ADD
#ADD https://community.download.adacore.com/v1/f3a99d283f7b3d07293b2e1d07de00e31e332325?filename=gnat-2021-20210519-x86_64-linux-bin  ./gnat-2021-20210519-x86_64-linux-bin
RUN wget -O gnat-2021-20210519-x86_64-linux-bin https://community.download.adacore.com/v1/f3a99d283f7b3d07293b2e1d07de00e31e332325?filename=gnat-2021-20210519-x86_64-linux-bin \
        && git clone https://github.com/AdaCore/gnat_community_install_script.git \
        && chmod +x gnat_community_install_script/install_package.sh \
        && chmod +x gnat-2021-20210519-x86_64-linux-bin \
        && gnat_community_install_script/install_package.sh ./gnat-2021-20210519-x86_64-linux-bin /opt/GNAT/gnat-x86-2021 \
        && cd \
        && rm -rf /gnat_tmp/ \
        && sed -i 's/# alias l=/alias l=/' ~/.bashrc \
        && sed -i 's/# export LS_OPTIONS/export LS_OPTIONS/' ~/.bashrc

WORKDIR /app/
ENV PATH="/opt/GNAT/gnat-x86-2021/bin:${PATH}"
ENV PATH="/root/asn1scc/asn1scc/bin/Debug/net7.0:${PATH}"
#ENTRYPOINT ["/bin/bash"]
```
  
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
If you decide to make any changes to the build.sh or build_fixed.sh files just remember to run the commands in order to keep the updated changes:
```
chmod -x build.sh
chmod -x build_fixed.sh 
```
**Expected Build Output:**
```
=== Cleaning and setting up directories ===
=== Compiling ASN.1 with T_ prefix ===
=== Verifying generated files ===
=== Copying runtime extensions ===
'/root/telemetry_project_asn1scc/src/asn1crt_mempool.c' -> '/root/telemetry_project_asn1scc/generated/asn1crt_mempool.c'
'/root/telemetry_project_asn1scc/src/asn1crt_mempool.h' -> '/root/telemetry_project_asn1scc/generated/asn1crt_mempool.h'
'/root/telemetry_project_asn1scc/src/asn1crt_stream.c' -> '/root/telemetry_project_asn1scc/generated/asn1crt_stream.c'
'/root/telemetry_project_asn1scc/src/asn1crt_stream.h' -> '/root/telemetry_project_asn1scc/generated/asn1crt_stream.h'
=== Compiling main program ===
=== Compiling memory benchmark ===
..
Memory benchmark built successfully. Run with: ./memory_benchmark [iterations]
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

### 3. Running the Main Program

```bash
./telemetry_program
```

### 4. Running the memory benchmark test:
```
./memory_benchmark
```

** Main Telemetry Program Output:**
```
===== Minimal Test =====
Status field type information:
Minimal encode successful: 15 bytes
Minimal decode failed: error 73
===== Telemetry Decoder Test =====
Raw test data as seen by decoder:
00 00 00 01 fa 00 00 00
49 c4 36 b1 38 82 0a Test Data (15 bytes):
00 00 00 01 fa 00 00 00
49 c4 36 b1 38 82 0a

DECODE FAILED (Error: 73)
Bytes processed: 15/15

===== Generated Test Vector Test =====
Encoding successful! Generated 15 bytes
Generated PER-encoded data (header-only):
00 00 00 01 fa 00 00 00
40 00 00 00 00 01 90
Decoding failed with error: 73
```

## Volume Mount Structure

| Host Path | Container Path |
|-----------|---------------|
| ~/asn1scc | /root/asn1scc |
| ~/telemetry_project_asn1scc | /root/telemetry_project_asn1scc |
