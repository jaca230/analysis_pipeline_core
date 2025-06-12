#!/bin/bash

# Resolve full paths
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_DIR=$(realpath "$SCRIPT_DIR/..")
BUILD_DIR="$BASE_DIR/build"

# Default flags
OVERWRITE=false
JOBS_ARG="-j"  # Use all processors by default

# Help message
show_help() {
    echo "Usage: ./build.sh [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -o, --overwrite         Remove existing build directory before building"
    echo "  -j, --jobs <number>     Number of processors to use (default: all)"
    echo "  -h, --help              Show this help message"
}

# Parse arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -o|--overwrite) OVERWRITE=true; shift ;;
        -j|--jobs)
            if [[ -n "$2" && "$2" != -* ]]; then
                JOBS_ARG="-j$2"
                shift 2
            else
                JOBS_ARG="-j"  # Use all processors
                shift
            fi
            ;;
        -h|--help) show_help; exit 0 ;;
        *) echo "[build.sh, ERROR] Unknown option: $1"; show_help; exit 1 ;;
    esac
done

# Build submodules first
echo "[build.sh, INFO] Building submodules..."
"$BASE_DIR/scripts/submodules/build_submodules.sh" "$@"

# Clean previous build if requested
if [ "$OVERWRITE" = true ]; then
    echo "[build.sh, INFO] Overwrite enabled: Cleaning previous build directory at $BUILD_DIR"
    "$SCRIPT_DIR/cleanup.sh"
fi

# Ensure build directory exists
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

# Run CMake and Make
echo "[build.sh, INFO] Configuring project with CMake in: $BUILD_DIR"
cmake "$BASE_DIR"

echo "[build.sh, INFO] Building project with: make $JOBS_ARG"
make $JOBS_ARG

echo "[build.sh, INFO] Build complete."
echo "[build.sh, INFO] Executables are in: $(realpath "$BUILD_DIR/bin")"
