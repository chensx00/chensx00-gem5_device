import sys
import os

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: python extract_verilator.py <file-prefix> <verilator_makefile_dir> <verilator_makefile>")
        sys.exit(1)
    f = os.popen(f"make -C {sys.argv[2]} -f {sys.argv[3]} debug-make")
    resolved_cpp = []
    for line in f:
        if line.startswith("VM_GLOBAL"):
            for c in line.split()[1:]:
                resolved_cpp.append(sys.argv[1] + c + '.cpp')
    print(' '.join(resolved_cpp))