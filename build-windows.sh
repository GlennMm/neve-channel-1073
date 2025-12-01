#!/bin/bash

# Build script for cross-compiling Neve 1073 to Windows using Docker

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/windows-build"

echo "=== Neve 1073 Windows Cross-Compiler ==="
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed. Please install Docker first."
    echo "  Ubuntu/Debian: sudo apt-get install docker.io"
    echo "  Then: sudo usermod -aG docker $USER"
    exit 1
fi

# Create output directory
mkdir -p "$OUTPUT_DIR"

echo "Building Docker image..."
docker build -t neve1073-windows-builder -f "${SCRIPT_DIR}/Dockerfile.windows" "${SCRIPT_DIR}"

echo ""
echo "Running Windows build..."
docker run --rm \
    -v "${OUTPUT_DIR}:/output" \
    neve1073-windows-builder

echo ""
echo "=== Build Complete ==="
echo "Windows binaries are in: ${OUTPUT_DIR}"
ls -la "${OUTPUT_DIR}" 2>/dev/null || echo "(directory may be empty if build failed)"
