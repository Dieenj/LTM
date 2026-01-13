#!/usr/bin/env bash
# Script để build & chạy Client (Qt/X11)

set -e

echo "[Script] Building client..."

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
CLIENT_DIR="$ROOT_DIR/Client"
BUILD_DIR="$CLIENT_DIR/build"

rm -rf "$BUILD_DIR"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$CLIENT_DIR"
cmake --build . -j$(nproc)

echo "[Script] Build successful! Starting client..."
./FileClient
