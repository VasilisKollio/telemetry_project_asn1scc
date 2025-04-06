#!/usr/bin/env python3
import os
import sys
import glob

RUNTIME_FILES = {
    'asn1crt', 'acn', 'partial', 'stream',
    'mempool', 'patched', 'encoding', 'rtkey'
}

def process_file(c_file, h_file):
    """Only add partial decoding to actual ASN.1 types"""
    basename = os.path.splitext(os.path.basename(c_file))[0]
    if any(basename.startswith(prefix) for prefix in RUNTIME_FILES):
        return
        
    with open(h_file, 'a') as f:
        f.write("\n/* Added by optimizer */\n")
        f.write("flag PartialDecode(BitStream* bs, void* val, PartialContext* ctx);\n")

def main():
    if len(sys.argv) != 2:
        print("Usage: script.py <generated_dir>")
        sys.exit(1)
        
    generated_dir = sys.argv[1]
    
    # Process only TelemetryModule files
    c_files = glob.glob(os.path.join(generated_dir, "TelemetryModule*.c"))
    for c_file in c_files:
        h_file = c_file[:-2] + '.h'
        if os.path.exists(h_file):
            process_file(c_file, h_file)
    
    print("Optimization complete")

if __name__ == "__main__":
    main()
