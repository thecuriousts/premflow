#!/bin/bash
set -e

BUILD_DIR="${BUILD_DIR:-build}"
ELOMAXZ_FLAGS=()
if [ -n "${ELOMAXZ_SOURCE_DIR:-}" ]; then
    ELOMAXZ_FLAGS=(-DELOMAXZ_SOURCE_DIR="$ELOMAXZ_SOURCE_DIR")
fi

echo "=== premflow Developer Build ==="
echo

echo "→ Configuring CMake..."
cmake -B "$BUILD_DIR" "${ELOMAXZ_FLAGS[@]}"

echo
echo "→ Building..."
cmake --build "$BUILD_DIR"

echo
echo "→ Running tests..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo
echo "✅ Build successful!"
echo
echo "You can now run:"
echo "  ./$BUILD_DIR/premflow"
echo "  cmake --install $BUILD_DIR --prefix ~/.local"
echo "  make install          # via Makefile wrapper"
